#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
void main(int argc, char * argv[] )
{
   FILE * source = fopen(argv[1],"r");//r 打开只读文件，该文件必须存在
   if(source==NULL){
	perror ("file no exsit!\n");
	return ;
   }
   else{
        //函数stat()用于获取一个文件路径的信息，并把获取到信息放到结构体struct stat 中
        //结构体struct stat 用于保存一个文件路径的信息
	   struct stat * statbuf=(struct stat *)malloc(sizeof(statbuf));
           stat(argv[2], statbuf); //判断目标路径是一个普通文件还是一个目录
	   if(S_ISDIR (statbuf->st_mode)) //目标是个目录
	    //宏S_ISDIR()用来判断一个文件路径是不是目录，是的话返回1，否则返回0
	   {
		   int i,j,k=0;//因为目标路径是一个文件夹，所以默认拷贝一个和源文件名字一样的名字
		//要找到源文件的名字，就要找源路径最后一个'/'后面的字符串就是源文件名！！
		   for(i=strlen(argv[1])-1;i>=0;i--)
		   {
		           if(argv[1][i]=='/') //找到了最后一个'/'的下标
			   {
			           break; //找到就退出 ，保留'/'下标为i
			   }
		   }
		   char source_file_name[128]={};//用来存储源文件的名字
		   for(j=i+1;j<strlen(argv[1]);j++)
		   {
			//把源路径最后一个'/'后面的字符串给source_file_name,它就是源文件名字
			   source_file_name[k++]=argv[1][j];
		   }
                //如果目标路径最后面没有‘/’，则需要加一个'/'
	           if(argv[2][strlen(argv[2])-1]!='/')
		   {
			   strcat (argv[2],"/");
		   }
		//把目标路径和源文件名拼接起来，成为一个目标文件名，并创建打开它
		   FILE * target = fopen(strcat (argv[2],source_file_name),"w+");
   //w+ 打开可读写文件，若文件存在则长度清零，即文件内容消失，若文件不存在则建立该文件
		   while(!feof(source))
		   {
			//把源文件内容全部传给目标文件
			   char  buf[10]={};
			   fread (buf,1,10,source);
			   fwrite (buf,1,10, target);
		   }
		   fclose (target);
	   }
	   else//目标路径是个文件
	   {
		   FILE * target = fopen(argv[2],"w+");
		   while(!feof(source))
		   {
		//把源文件内容全部传给目标文件
			   char  buf[10]={};
			   fread (buf,1,10,source);
			   fwrite (buf,1,10, target);
		   }
                   fclose (target);
	   }
	   fclose (source);
   }
}

