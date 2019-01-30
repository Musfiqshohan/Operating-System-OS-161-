/**
* This file is added for assignment 2
**/
#ifndef __FILE_H
#define __FILE_H

#include <types.h>
#include <synch.h>
#include <vnode.h>
/* Some limits  */

/* This is the maximum number of files that can be opened per
 * process */
#define MAX_PROCESS_OPEN_FILES  16

/* This is the maximum number of files that can be open in the system
 * at any one time */
#define MAX_SYSTEM_OPEN_FILES   64 
struct vnode;
struct lock;
/*
 * Put your function declarations and data types here ...
 */

struct File {
	struct vnode *v_ptr;
    int open_flags;
	int references;
	struct lock *flock;
    off_t offset;
};


int syscall_open(userptr_t filename, int flags, int *ret);
int syscall_read(int filehandler, userptr_t buf, size_t size, int *ret);
int syscall_write(int filehandler, userptr_t buf, size_t size, int *ret);
int syscall_close(int filehandler);
int sys_dup2(int oldfd, int newfd);
int syscall_lseek(int fd, off_t pos, userptr_t whence_ptr, int32_t *ret);
int check_SEEK(int whence,off_t pos,struct File *file, struct stat stats, int32_t *ret);
/* Place your data-structures here ... */

#endif
