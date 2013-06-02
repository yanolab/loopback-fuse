#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 28
#endif

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include <linux/limits.h>

static const char *loopback_path = "/tmp/loop";

static void resolve_loopback_path(char *buf, const char *path) {
  strcpy(buf, loopback_path);
  strcat(buf, path);

  printf("loopback path: %s to %s\n", path, buf);
}

static int loopback_access(const char *path, int mode) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return access(buf, mode);
}

static int loopback_create(const char *path, mode_t mode, fuse_file_info *fi) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  fi->fh = creat(buf, mode);

  return (int)fi->fh != -1 ? 0 : -errno;
}

static int loopback_flush(const char *path, fuse_file_info *fi) {
  // ioctl??
  return fsync(fi->fh);
}

static int loopback_fsync(const char *path, int datasync, fuse_file_info *fi) {
  return datasync != 0 ? fdatasync(fi->fh) : fsync(fi->fh);
}

static int loopback_getattr(const char *path, struct stat *stbuf) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return stat(buf, stbuf) == -1 ? -errno : 0;
}

static int loopback_link(const char *from, const char *to) {
  char buf1[PATH_MAX], buf2[PATH_MAX];
  resolve_loopback_path(buf1, from);
  resolve_loopback_path(buf2, to);

  return link(buf1, buf2);
}

static int loopback_read(const char *path, char *buffer, size_t size, off_t offset, fuse_file_info *fi) {
  lseek(fi->fh, offset, SEEK_SET);
  return read(fi->fh, buffer, size);
}

static int loopback_write(const char *path, const char *buffer, size_t size, off_t offset, fuse_file_info *fi) {
  lseek(fi->fh, offset, SEEK_SET);
  return write(fi->fh, buffer, size);
}

static int loopback_mkdir(const char *path, mode_t mode) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return mkdir(buf, mode);
}

static int loopback_mknod(const char *path, mode_t mode, dev_t dev) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return mknod(buf, mode, dev);
}

static int loopback_open(const char *path, fuse_file_info *fi) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  fi->fh = open(buf, O_RDWR | O_APPEND);

  return (int)fi->fh != -1 ? 0 : -errno;
}

static int loopback_readlink(const char *path, char *buffer, size_t size) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return readlink(buf, buffer, size);
}

static int loopback_release(const char *path, fuse_file_info *fi) {
  return close(fi->fh);
}

static int loopback_rename(const char *from, const char *to) {
  char buf1[PATH_MAX], buf2[PATH_MAX];
  resolve_loopback_path(buf1, from);
  resolve_loopback_path(buf2, to);

  return rename(buf1, buf2);
}

static int loopback_rmdir(const char *path) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return rmdir(buf);
}

static int loopback_statfs(const char *path, struct statvfs *stvfsbuf) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return statvfs(buf, stvfsbuf);
}

static int loopback_symlink(const char *from, const char *to) {
  char buf1[PATH_MAX], buf2[PATH_MAX];
  resolve_loopback_path(buf1, from);
  resolve_loopback_path(buf2, to);

  return symlink(buf1, buf2);
}

static int loopback_truncate(const char *path, off_t offset) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return truncate(buf, offset) == 0 ? 0 : -errno;
}

static int loopback_unlink(const char *path) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return unlink(buf);
}

static int loopback_utime(const char *path, struct utimbuf *times) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return utime(buf, times);
}

static int loopback_chmod(const char  *path, mode_t mode) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return chmod(buf, mode);
}

static int loopback_chown(const char *path, uid_t uid, gid_t gid) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  return chown(buf, uid, gid);
}

static int loopback_ftruncate(const char *path, off_t offset, fuse_file_info *fi) {
  return ftruncate(fi->fh, offset);
}

static int loopback_fgetattr(const char *path, struct stat *stbuf, fuse_file_info *fi) {
  return fstat(fi->fh, stbuf);
}

static int loopback_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, fuse_file_info *fi) {
  char buf[PATH_MAX];
  resolve_loopback_path(buf, path);

  struct dirent *dirent;
  DIR *dp = opendir(buf);
  while((dirent = readdir(dp)) != NULL) {
    filler(buffer, dirent->d_name, NULL, 0);
  }
  closedir(dp);

  return 0;
}

int main(int argc, char *args[]) {
  struct fuse_operations *operations;
  operations = (fuse_operations *) malloc(sizeof(fuse_operations));
  memset(operations, 0, sizeof(fuse_operations));

  operations->access = loopback_access;
  operations->create = loopback_create;
  operations->flush = loopback_flush;
  operations->fsync = loopback_fsync;
  operations->getattr = loopback_getattr;
  operations->link = loopback_link;
  operations->read = loopback_read;
  operations->write = loopback_write;
  operations->mkdir = loopback_mkdir;
  operations->mknod = loopback_mknod;
  operations->open = loopback_open;
  operations->readlink = loopback_readlink;
  operations->release = loopback_release;
  operations->rename = loopback_rename;
  operations->rmdir = loopback_rmdir;
  operations->statfs = loopback_statfs;
  operations->symlink = loopback_symlink;
  operations->utime = loopback_utime;
  operations->truncate = loopback_truncate;
  operations->unlink = loopback_unlink;
  operations->chmod = loopback_chmod;
  operations->chown = loopback_chown;
  operations->ftruncate = loopback_ftruncate;
  operations->fgetattr = loopback_fgetattr;
  operations->readdir = loopback_readdir;

  /*
  operations->setxattr = loopback_setxattr;
  operations->getxattr = loopback_getxattr;
  operations->listxattr = loopback_listxattr;
  operations->removexattr = loopback_removexattr;
  operations->opendir = loopback_opendir;
  operations->releasedir = loopback_releasedir;
  operations->fsyncdir = loopback_fsyncdir;
  operations->destroy = loopback_destroy;
  operations->lock = loopback_lock;
  */

  printf("fuse_main:\n");
  printf("\targc=%d\n", argc-1);
  for(int i = 0; i < argc; i++) {
    printf("\targs[%d]=%s\n", i, args[i]);
  }

  return fuse_main(argc-1, args+1, operations, NULL);
}
