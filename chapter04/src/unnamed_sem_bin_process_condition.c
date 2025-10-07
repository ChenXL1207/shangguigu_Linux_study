#include "common.h"
#include <sys/mman.h>
#include <sys/wait.h>

int main(int argc, char const *argv[])
{
	char *shm_value_name = "/unnamed_sem_bin_process_condition";
	//创建共享内存
	int shm_fd = shm_open(shm_value_name, O_CREAT | O_RDWR, 0666);
	// 调整共享内存大小
	ftruncate(shm_fd, sizeof(int));
	// 映射共享内存
	int *shm_value = (int *)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	// 初始化共享内存
	*shm_value = 0;

	int pid = fork();
	if(pid > 0){
		// 父进程
		int tmp = *shm_value + 1;
		sleep(1);
		*shm_value = tmp;

		// 等待子进程执行完毕
		waitpid(pid, NULL, 0);
		printf("父进程执行完毕，共享内存值为：%d\n", *shm_value);

		// 仅父进程负责删除命名共享内存对象
		if(shm_unlink(shm_value_name) == -1){
			perror("shm_unlink");
			exit(1);
		}
	}else if(pid == 0){
		// 子进程
		int tmp = *shm_value + 1;
		sleep(1);
		*shm_value = tmp;
		printf("子进程执行完毕，共享内存值为：%d\n", *shm_value);
	}else{
		perror("fork");
		exit(1);
	}

	// 释放共享内存
	if(munmap(shm_value, sizeof(int)) == -1){
		perror("munmap");
		exit(1);
	}
	if(close(shm_fd) == -1){
		perror("close");
		exit(1);
	}
	return 0;
}