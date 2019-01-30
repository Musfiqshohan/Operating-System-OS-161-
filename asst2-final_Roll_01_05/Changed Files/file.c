#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <kern/stat.h>
#include <kern/seek.h>
#include <limits.h>
#include <lib.h>
#include <uio.h>
#include <thread.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>
#include <vnode.h>
#include <file.h>
#include <proc.h>
#include <syscall.h>
#include <copyinout.h>
#include <machine/trapframe.h>
/*
 * Add your file-related functions here ...
 */



static int open(char *filename, int flags, int descriptor);
static int close(int filehandler, struct proc *proc);
static int open_file_cnt=0;





int syscall_open(userptr_t filename, int flags, int *ret) {

	
	size_t copy_parameter;
	int newfile_index=3,result,err;
	if (filename == NULL) {
		return EFAULT;
	}
	char *cur_filename = kmalloc((PATH_MAX)*sizeof(char));
	if (cur_filename == NULL) {
		return ENFILE;
	}
	result = copyinstr(filename, cur_filename, PATH_MAX, &copy_parameter);
	if (result) {
		kfree(cur_filename);
		return result;
	}
	for (; newfile_index < MAX_PROCESS_OPEN_FILES; newfile_index++) {
		if (curproc->process_file_table[newfile_index] == NULL) {
			break;
		}
	}
	if (newfile_index == MAX_PROCESS_OPEN_FILES) {
		kfree(cur_filename);
		return EMFILE;
	}
	if (open_file_cnt>=MAX_SYSTEM_OPEN_FILES)
	{
		kfree(cur_filename);
		return ENFILE;
	}

	err = open(cur_filename, flags, newfile_index);
	if(err){
		kfree(cur_filename);
		return err;
	}

	*ret = newfile_index;
	open_file_cnt++;
	return 0;
}

int syscall_close(int filehandler) { return close(filehandler, curproc);}

int syscall_read(int filehandler, userptr_t buf, size_t size, int *ret) {
	struct iovec iov;
	struct uio myuio;
	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !curproc->process_file_table[filehandler]) {
		return EBADF;
	}
	struct File *file = curproc->process_file_table[filehandler];

	int how = file->open_flags & O_ACCMODE;
	if (how == O_WRONLY) {
		return EBADF;
	}
	lock_acquire(file->flock);
	off_t old_offset = file->offset;

	uio_uinit(&iov, &myuio, buf, size, file->offset, UIO_READ);

	int result = VOP_READ(file->v_ptr, &myuio);
	if (result) {
		lock_release(file->flock);
		return result;
	}

	file->offset = myuio.uio_offset;
	*ret = file->offset - old_offset;
	lock_release(file->flock);
	return 0;
}

int syscall_write(int filehandler, userptr_t buf, size_t size, int *ret) {
	struct iovec fiovec;
	struct uio fuio;
	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !curproc->process_file_table[filehandler]) {		
		return EBADF;
	}
	struct File *file = curproc->process_file_table[filehandler];
	int how = file->open_flags & O_ACCMODE;
	if (how == O_RDONLY) {
		return EBADF;
	}
	lock_acquire(file->flock);
	off_t old_offset = file->offset;
	uio_uinit(&fiovec, &fuio, buf, size, file->offset, UIO_WRITE);
	int result = VOP_WRITE(file->v_ptr, &fuio);
	if (result) {
		lock_release(file->flock);
		return result;
	}
	file->offset = fuio.uio_offset;
	*ret = file->offset - old_offset;
	lock_release(file->flock);
	return 0;
}

int sys_dup2(int oldfd, int newfd) {
	if(oldfd < 0 || oldfd >= MAX_PROCESS_OPEN_FILES || newfd < 0 || newfd >= MAX_PROCESS_OPEN_FILES || !curproc->process_file_table[oldfd]) {
		return EBADF;
	}
	if(oldfd == newfd){
		return 0;
	}
	if(curproc->process_file_table[newfd]){
		int result = syscall_close(newfd);
		if(result){
			return result;
		}
	}
	struct File *file = curproc->process_file_table[newfd] = curproc->process_file_table[oldfd];
	lock_acquire(file->flock);
	file->references++;
	lock_release(file->flock);
	return 0;
}




int check_SEEK(int whence,off_t pos,struct File *file, struct stat stats, int32_t *ret)
{

switch(whence){
		case SEEK_SET:
			if(pos < 0){
				return EINVAL;
			}
			lock_acquire(file->flock);
			*ret = file->offset = pos;
			lock_release(file->flock);
			break;

		case SEEK_CUR:
			lock_acquire(file->flock);
			if(file->offset + pos < 0){
				lock_release(file->flock);
				return EINVAL;
			}
			*ret = file->offset += pos;
			lock_release(file->flock);
			break;
		
		case SEEK_END:
			if(stats.st_size + pos < 0){
				return EINVAL;
			}
			lock_acquire(file->flock);
			*ret = file->offset = stats.st_size + pos; 
			lock_release(file->flock);
			break;

		default:
			return EINVAL;
	}
	
	return 0;





}

int syscall_lseek(int fd, off_t pos, userptr_t whence_ptr, int32_t *ret) {
	struct File *file;
	struct stat stats;
	int whence;
	if(fd < 0 || fd > MAX_PROCESS_OPEN_FILES || !(file = curproc->process_file_table[fd])){
		return EBADF;
	}
	if(!VOP_ISSEEKABLE(file->v_ptr)){
		return ESPIPE;
	}
	int result = VOP_STAT(file->v_ptr, &stats);
	if(result){
		return result;
	}
	result = copyin(whence_ptr, &whence, sizeof(int));
	if(result) {
		return result;
	}

	
	return check_SEEK(whence,pos,file,stats,ret);
	
}

//These functions are called from syscall_open to implement file open.

static int open(char *filename, int flags, int descriptor){
	struct File *file = kmalloc(sizeof(struct File*));
	int result;
	struct vnode *vn;
	if(!file){
		return ENFILE;
	}
 
	result = vfs_open(filename, flags, 0, &vn);
	if (result) {
		kfree(file);
		return result;
	}

	file->flock = lock_create("lock create");
	if(!file->flock) {
		vfs_close(file->v_ptr);
		kfree(file);
		return ENFILE;
	}
	file->offset = 0;
	file->open_flags = flags;
	file->references = 1;
	file->v_ptr=vn;
	curproc->process_file_table[descriptor] = file;

	return 0;
}
static int close(int filehandler, struct proc *proc) {
	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !proc->process_file_table[filehandler]) {
		return EBADF;
	}

	struct File *file = proc->process_file_table[filehandler];

	lock_acquire(file->flock);
	proc->process_file_table[filehandler] = NULL;
	file->references --;
	if(file->references<=0) {
		lock_release(file->flock);
		vfs_close(file->v_ptr);
		lock_destroy(file->flock);
		kfree(file);
	}
	else{
		lock_release(file->flock);
	}
	open_file_cnt--;
	return 0;
}
