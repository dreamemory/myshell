#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

void del_stat(const char* name)
{
	DIR* fl = opendir(name);
	chdir(name);

	for(struct dirent* dir = readdir(fl); NULL!=dir;)
	{
		if(dir->d_type == DT_DIR)
		{
		    //在遍历的时候忽略代表当前目录的. 和代表上一目录的..
			if(strcmp(dir->d_name,"..")&&strcmp(dir->d_name,"."))
				del_stat(dir->d_name);
		}
		remove(dir->d_name);
		dir = readdir(fl);
	}
	chdir("..");
	remove(name);//当一个目录中的所有内容全部被删除之后，便删除该目录
}

int main(int argc,char *argv[])
{
	if(3 != argc)
	{
		printf("User:./rm -rf xxx\n");//删除当前目录下的所有文件和目录
		return -1;
	}
    //函数stat()用于获取一个文件路径的信息，并把获取到信息放到结构体struct stat 中
    //结构体struct stat 用于保存一个文件路径的信息
	struct stat sta;
	if(0 > stat(argv[2],&sta))
	{
		perror("open");//perror()用来讲将上一个函数发生错误的原因输出到标准设备(stderr)
		//错误原因为已经打开文件
		return -1;
	}
    //宏S_ISDIR()用来判断一个文件路径是不是目录，是的话返回1，否则返回0
    //宏S_ISREG()用来判断一个文件路径是不是一个普通文件，是的话返回1，否则返回0
	if(!S_ISDIR(sta.st_mode))//目标是一个文件
	{
		char temp;//定义一个字符变量
		printf("该文件是一个文件，是否删除(y)、(n):");
		scanf("%c",&temp);
		if(temp == 'n')return 0;//如果用户填的是n的话，就不删除
		remove(argv[2]);
		return 0;
	}

	del_stat(argv[2]);
}
