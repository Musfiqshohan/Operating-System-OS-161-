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

// TODO: Take in 3rd argument register and give to vfs_open (mode_t)
// Although this is not even implemented in OS161 so whatever...
/*
int sys_fork(struct trapframe *tframe,int *retval)
{
	char name[]="Omi";
	struct proc *nproc=proc_create_runprogram(name);
	nproc->par=curproc;
	nproc->count=0;
	filetable_init(nproc);
	
	
	curproc->childlist[count]=nproc;
	curproc->count++;
	return 0;
}
*/

static int open_file_cnt=0;
static int open(char *filename, int flags, int descriptor){
	struct File *file = kmalloc(sizeof(struct open_file*));
	int result;
	if(!file){
		return ENFILE;
	}
 
	result = vfs_open(filename, flags, 0, &file->v_ptr);
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
	curproc->descriptor_table[descriptor] = file;

	return 0;
}
static int open2(struct proc *newproc,char *filename, int flags, int descriptor){
	struct File *file = kmalloc(sizeof(struct File*));
	int result;
	if(!file){
		return ENFILE;
	}
 
	result = vfs_open(filename, flags, 0, &file->v_ptr);
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
	newproc->descriptor_table[descriptor] = file;

	return 0;
}
int filetable_init(struct proc *newproc)
{
	int result;
	//kprintf("1\n");
	if(newproc->descriptor_table[0]==NULL)
	{
		char temp1[]="dumb";
		result=open2(newproc,temp1,O_RDONLY,0);
		if(result) {
			kprintf("fileatable_init failed\n");
			kfree(temp1);
			return EINVAL;
		}	
	}
	//kprintf("2\n");
	if(newproc->descriptor_table[1]==NULL)
	{
		char temp2[]="dumb";
		result=open2(newproc,temp2,O_WRONLY,1);
		if(result) {
			kprintf("fileatable_init failed\n");
			kfree(temp2);
			return EINVAL;
		}
		
	}
	
	//kprintf("1\n");
	if(newproc->descriptor_table[2]==NULL)
	{
		char temp3[]="dumb";
		result=open2(newproc,temp3,O_WRONLY,2);
		if(result) {
			kprintf("fileatable_init failed\n");
			kfree(temp3);
			return EINVAL;
		}
		
	}
	return 0;
}

int close(int filehandler, struct proc *proc) {
	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !proc->descriptor_table[filehandler]) {
		return EBADF;
	}

	struct File *file = proc->descriptor_table[filehandler];

	lock_acquire(file->flock);
	proc->descriptor_table[filehandler] = NULL;
	file->references -= 1;

	if(!file->references) {
		lock_release(file->flock); //This is the last reference to this open file
		vfs_close(file->v_ptr);
		lock_destroy(file->flock);
		kfree(file);
	}
	else{
		lock_release(file->flock);
	}
	open_file_cnt--;
//	kprintf("\n***************Close SuccessFul**********************\n");
	return 0;
}

void open_std(void) {
	char con1[] = "con:", con2[] = "con:";
	KASSERT(open(con1, O_WRONLY, 1) == 0); 
	KASSERT(open(con2, O_WRONLY, 2) == 0);
}

int sys_open(userptr_t filename, int flags, int *ret) {
	size_t got;
	int i=3,result,err;
	if (filename == NULL) {
		return EFAULT;
	}
	char *kfilename = kmalloc((PATH_MAX)*sizeof(char));
	if (kfilename == NULL) {
		return ENFILE;
	}
	result = copyinstr(filename, kfilename, PATH_MAX + 1, &got);
	if (result) {
		kfree(kfilename);
		return result;
	}
	for (; i < MAX_PROCESS_OPEN_FILES; i++) {
		if (curproc->descriptor_table[i] == NULL) {
			break;
		}
	}
	if (i == MAX_PROCESS_OPEN_FILES) {
		kfree(kfilename);
		return EMFILE;
	}
	if (open_file_cnt>=MAX_SYSTEM_OPEN_FILES)
	{
		kfree(kfilename);
		return ENFILE;
	}

	err = open(kfilename, flags, i);
	if(err){
		kfree(kfilename);
		return err;
	}

	*ret = i;
	open_file_cnt++;
	//kprintf("\n***************Open SuccessFul**********************\n");
	return 0;
}

int sys_close(int filehandler) {
	return close(filehandler, curproc);
}

int sys_read(int filehandler, userptr_t buf, size_t size, int *ret) {
//	kprintf("%d\n",filehandler);
	struct iovec iov;
	struct uio myuio;
	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !curproc->descriptor_table[filehandler]) {
		return EBADF;
	}
	struct File *file = curproc->descriptor_table[filehandler];

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
	
	//kprintf("\n***************Read SuccessFul**********************\n");
	return 0;
}

int sys_write(int filehandler, userptr_t buf, size_t size, int *ret) {
//	kprintf("\nWrite reach 0\n");
//	kprintf("%d %d\n",filehandler,MAX_PROCESS_OPEN_FILES);
	struct iovec iov;
	struct uio myuio;
	if(filehandler==1 || filehandler==2)
	{
		//kprintf("%s\n",*buf);
	}
	if(filehandler < 0 || filehandler >= MAX_PROCESS_OPEN_FILES || !curproc->descriptor_table[filehandler]) {
		kprintf("\nWrite reach 0.2\n");		
		return EBADF;
	}
/*
	if(!curproc->descriptor_table[filehandler])
	{
		char dumb[] = "dumb.txt";
		open(dumb, 2, filehandler);
	}
*/	
	//kprintf("Write reach 1");
	struct File *file = curproc->descriptor_table[filehandler];
	int how = file->open_flags & O_ACCMODE;
//	kprintf("\nhow %d\n",how);
	if (how == O_RDONLY) {
		return EBADF;
	}
	
	
//	kprintf("Write reach 2");
	lock_acquire(file->flock);
	off_t old_offset = file->offset;

	uio_uinit(&iov, &myuio, buf, size, file->offset, UIO_WRITE);

	int result = VOP_WRITE(file->v_ptr, &myuio);
	if (result) {
		lock_release(file->flock);
		return result;
	}
//	kprintf("Write reach 3");
	file->offset = myuio.uio_offset;
	*ret = file->offset - old_offset;
	lock_release(file->flock);
	
	//kprintf("\n***************Write SuccessFul**********************\n");
	return 0;
}

int sys_dup2(int oldfd, int newfd) {
	if(oldfd < 0 || oldfd >= MAX_PROCESS_OPEN_FILES ||
			newfd < 0 || newfd >= MAX_PROCESS_OPEN_FILES ||
			!curproc->descriptor_table[oldfd]) {
		return EBADF;
	}

	if(oldfd == newfd){
		return 0;
	}

	if(curproc->descriptor_table[newfd]){
		int result = sys_close(newfd);
		if(result){
			return result;
		}
	}

	struct File *file = curproc->descriptor_table[newfd] = curproc->descriptor_table[oldfd];
	lock_acquire(file->flock);
	file->references++;
	lock_release(file->flock);
	//kprintf("\n***************dup2 SuccessFul**********************\n");
	return 0;
}

int sys_lseek(int fd, off_t pos, userptr_t whence_ptr, off_t *ret) {
	struct File *file;

	if(fd < 0 || fd > MAX_PROCESS_OPEN_FILES || !(file = curproc->descriptor_table[fd])){
		return EBADF;
	}

	if(!VOP_ISSEEKABLE(file->v_ptr)){
		return ESPIPE;
	}

	struct stat stats;
	int result = VOP_STAT(file->v_ptr, &stats);
	if(result){
		return result;
	}

	int whence;
	result = copyin(whence_ptr, &whence, sizeof(int));
	if(result) {
		return result;
	}

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

