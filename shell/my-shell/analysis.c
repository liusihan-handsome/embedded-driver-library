/*这个文件主要完成命令行的解析*/
//基础命令:ls/clear/help/reboot
#include "project.h"
#include "string.h"
//函数指针
typedef int(*deal_cmd_t)(int argc,char** argv);

 typedef struct{
     char *name;
     deal_cmd_t cmd_entry;
 }cmd;
 int test_cmd(int argc,char **argv)
 {
     printf("%s",__func__);
     for(int i=0;i<argc;i++)
     {
         printf("%s",argv[i]);
     }
 }
 int clear_cmd(int argc,char **argv)
 {
      printf("%s",__func__);
 }
 int reboot_cmd(int argc,char **argv)
 {
      printf("%s",__func__);
 }
 int help_cmd(int argc,char **argv)
 {
      printf("%s",__func__);
 }
int get_list_len();
const char *get_cmd_name(int index);
 int ls_cmd(int argc,char **argv)
 {
     //printf("%s",__func__);
     for(int i=0;i<get_list_len();i++)
     {
         printf("%s\r\n",get_cmd_name(i));
     }
 }
 cmd cmd_list[]={
     {"test",test_cmd},
     {"clear",clear_cmd},
     {"reboot",reboot_cmd},
     {"help",help_cmd},
     {"ls",ls_cmd}
 };
 
  int get_list_len()
 {
     return ITEM_NUM(cmd_list);
 }
 
 const char *get_cmd_name(int index)
{
    if (index < 0 || index >= get_list_len())
        return NULL;
    return cmd_list[index].name;
}
 //切割字符串
//max 最多解析几个参数
//str 需要处理的字符串
//result 存放处理的结果
//delimiters 分隔符 " \t\n\r"
static int split_string_n(int max,char *str, char **result, const char *delimiters) 
{
    int count = 0;
    char *token;
    char *saveptr; // 用于保存上下文的指针

    // 使用 strtok_r 函数拆分字符串
    token = strtok_r(str, delimiters, &saveptr);

    while (token != NULL) {
        result[count] = token; // 保存当前的 token
        count++;
                if( count>=max ) break;
        // 获取下一个 token
        token = strtok_r(NULL, delimiters, &saveptr);
    }

    return count;

}
static uint8_t sta=0;
#define rx_buf_len 10
char rx_buf[rx_buf_len];
uint8_t rx_len;

 void cmd_line_work(void)
 {
    uint32_t ret;
     uint8_t *buf_t=NULL;
     char *argv[rx_buf_len];
     int argc=0;
     switch(sta)
     {
         case 0:
         memset(rx_buf,0,rx_buf_len);
         rx_len=0;
         sta++;
         break;
         case 1:
             buf_t = (uint8_t *)( &(rx_buf[rx_len])  );
            ret=loopbuf_read(buf,buf_t,1);
         if(ret==0) break;
         rx_len+=ret;
         
         //需要判断缓冲区是否溢出
			if( rx_len >= rx_buf_len )
			{//成立，说明缓冲区满了
				sta = 0;
				break;
			}
            
            switch(buf_t[0])
            {
                
                case 0x09://TAB键
					//检索适配命令，并打印出来提示
				
					break;
				
				case 0x7F:
				case '\b':					
					//需要先搞懂 rx_len 代表什么含义
					//rx_len 此时，代表了收到退格符之后，下一个接收数据的数组下标
					rx_len --;
					//rx_len 此时，代表了退格符的数组下标
					rx_buf[ rx_len ] = 0;	//清除退格符号
					
					//按下退格键的目的，是为了把原有的数据，清掉一个
					
					//先判断原来有没有数据
					if( rx_len == 0 )
					{//说明，原本是没有数据的
						
					}
					else
					{//说明，原本是有数据的
						rx_len--;
						//rx_len 此时,代表了原有数据的最后一个字符的数组下标
						rx_buf[ rx_len ] = '\0';	//清除原有数据的最后一个字符 
						
					}
					printf("\r\n>%s",rx_buf);
					
				
					
					break;
				
				case '\n':
					//有些串口工具发送回车换行，\n不是我们想要的
					//如果命令首部是\n，则直接丢弃。
					if( rx_len == 1 )	
						rx_len = 0;
					break;
					
				case '\r':
					argc = split_string_n( rx_buf_len,rx_buf,argv," \t\n\r" );
					if( argc != 0 )
					{
						
						//在命令表里面匹配 argv[0]
						for( int i=0;i< (ITEM_NUM( cmd_list ) );i++ )
						{
							if( 0 == strncmp( rx_buf,cmd_list[i].name ,strlen( cmd_list[i].name ) ) )
							{//成立，说明字符串一致，找到相应的命令了
								printf("\r\n");
								cmd_list[i].cmd_entry( argc,argv );
								
								sta = 0;
								return;
							}

						}
						printf("\r\n\tunknow");
					}
			
					sta = 0;
		
				
					break;
				
				default:
					//直接打印出来(回显)
					printf("%c",buf_t[0] );
					break;
				
			}
			
			break;
		
		default:
			break;
                
                
            }
     }
 