#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include "font.h"
#include <stdlib.h>
#include <stdbool.h>
#include <linux/input.h>

/*
重要提醒：屎山代码,维护不易,介意勿喷！！！
重要提醒：屎山代码,维护不易,介意勿喷！！！
重要提醒：屎山代码,维护不易,介意勿喷！！！
重要的事情说三遍！！！

构建版本：V3.2
构建日期：2024-12-23 13:45:50 UTC+8
维护者：KD

更新日志：
1.修复了显示图片时，图片的颜色数据读取错误的问题
2.修复了多次点击退出游戏，再次打开游戏界面跳转错误的问题
3.优化注释显示内容
4.优化各循环代码结构，减少不必要的变量，提高系统稳定性
5.新增首页开屏的功能
6.新增显示文本框的功能
7.新增首页未打开游戏的提示功能
8.新增游戏初始化环节
9.新增游戏账号和密码输入功能
10.新增游戏退出功能
11.新增桌面回跳功能

温馨提示：为实现良好的游戏体验，本人在主界面、游戏界面等
         加入了许多嵌套循环代码，在未熟知代码的前提下，
         贸然修改循环结构将导致程序的异常崩溃，
         因此，在修改代码结构时，应当充分考虑代码的逻辑，
         确保修改后的代码能够正常运行。
*/

//全局变量定义
int app_click = 0;  //判断是否正确触碰到游戏 0：点击到了游戏 1：未点击游戏+
int input_x, input_y; // 触摸屏的坐标


//图片显示实现
int show_bmp_to_lcd(char *picname)
{
	//打开lcd、bmp文件
	int lcd_fd = open("/dev/fb0", O_RDWR);
	int bmp_fd = open(picname, O_RDWR);
	if (lcd_fd == -1)
	{
		printf("无法打开屏幕.\n");
		return -1;
	}
	if (bmp_fd == -1)
	{
		printf("无法打开照片.\n");
		return -2;
	}

    //获取bmp图片的颜色数据   bmpbuf:存放bmp图片的颜色数据
	char bmp_buf[800*480*3];
	lseek(bmp_fd, 54, SEEK_SET);	// 偏移文件的光标
	read(bmp_fd, bmp_buf, 800*480*3);

	//写入数据
	//申请映射空间
	int *mmap_start = mmap(NULL, 800*480*4, PROT_READ|PROT_WRITE, MAP_SHARED, lcd_fd, 0);
	if (mmap_start == (void *)-1)
	{
		printf("申请映射空间失败.\n");
		return -3;
	}
	//使用映射空间
	int n=0;
	for(int y=0; y<480; y++)
	{
		for(int x=0; x<800; x++, n+=3)
		{
			*(mmap_start+800*(479-y)+x) =   bmp_buf[n+0]<<0|
									        bmp_buf[n+1]<<8|
									        bmp_buf[n+2]<<16|
									        0<<24;
		}
	}
	//撤销映射空间 --》 内存泄露
	munmap(mmap_start, 800*480*4);

	//关闭文件
	close(lcd_fd);
	close(bmp_fd);
	return 0;
}

//触摸功能实现
int input_fun()
{
    int input_fd = open("/dev/input/event0", O_RDWR);       //打开触摸屏文件
    struct input_event input_buf;
    int x,y;    
    while(1)
    {
        read(input_fd, &input_buf, sizeof(input_buf));   //读取触摸屏数据:input_buf
        if(input_buf.type == EV_ABS && input_buf.code == ABS_X)   //判断是否是触摸屏事件
        {
            x = input_buf.value;   //获取触摸屏的x坐标
        }
        if(input_buf.type == EV_ABS && input_buf.code == ABS_Y)   //判断是否是触摸屏事件
        {
            y = input_buf.value;   //获取触摸屏的y坐标
        }
        if(input_buf.type == EV_KEY && input_buf.code == BTN_TOUCH && input_buf.value == 0)   //判断是否是触摸屏按下事件
        {
            x = x*800/1024;   //将触摸屏的坐标转换为LCD的坐标
            y = y*480/600;
            input_x = x;
            input_y = y;
            printf("x=%d,y=%d\n",input_x,input_y);   //打印坐标值
            break;
         }
    }
    close(input_fd);
    return 0;
}

//账号和密码文本框功能实现
int account_password_textbox_fun()
{
    //申请内存映射
    int lcd_fd = open("/dev/fb0", O_RDWR);
    int *p = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
    //初始化字库 f:字库句柄 "simkai.ttf":字库名字
    font *f = fontLoad("simkai.ttf");
    //设置字体大小 32:字体大小
    fontSetSize(f,32);
    //设置文本框大小 300*40：表示文本框大小 4：表示色素    A R G B
    //账号密码文本框 account_textbox：表示账号文本框 password_textbox：表示密码文本框
    //account_buf：表示账号文本框显示的字符串
    //password_buf：表示密码文本框显示的字符串
    //hide_password_buf：表示密码文本框显示的字符串，隐藏密码
    //判断变量：m
    char account_buf[128]={0};
    char password_buf[128]={0};
    char hidepassword_buf[128]={0}; //隐藏密码
    bitmap *account_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //白色
    bitmap *password_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //白色
    int m=0;    //判断变量m
    int flag=0; //flag=0表示账号输入，flag=1表示密码输入   
    //将字体输入到文本框内 account_textbox:表示文本框 password_textbox:表示密码文本框 0,0：表示字体显示的位置
    //fontPrint(f,account_textbox,0,0,account_buf,getColor(0,0,0,0),0); 
    //fontPrint(f,password_textbox,0,0,password_buf,getColor(0,0,0,0),0); 
    //将文本框显示到LCD上 300,190：文本框在LCD上显示的起始位置
    show_font_to_lcd(p,81,200,account_textbox);
    show_font_to_lcd(p,81,245,password_textbox);

    /*
    //方法一：通过回车输入账号密码
    while(1)
    {
        //AO表示中间值，用来判断输入的内容
        char AO=getchar();
        if(AO =='\n')   //跳过回车
        {
            AO=' ';
            continue;
        }
        else if(AO == '`')  //切换密码
        {
            flag=1;
            m=0;//清空数组下标，不影响账号密码输入
        }

        //判断账号密码长度是否超过界限
        if(m<10)    
        {
            if(flag==0)
            {
                account_buf[m]=AO;
                m++;
            }
            else
            {
                password_buf[m]=AO;
                m++;
            }
        }

        //显示修改
        if(flag==0)
        {
            fontPrint(f,account_textbox,5,4,account_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,150,190,account_textbox);
        }
        else
        {
            fontPrint(f,password_textbox,5,4,password_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,150,250,password_textbox);
        }
    }
    */

    //方法二：通过关闭缓冲区实现输入账号密码
    while(1)
    {
        //关闭缓冲区(实现不敲回车输入账号密码)
        system("stty -icanon");
        //AO表示中间值，用来判断输入的内容
        char AO=getchar();
        if(AO == '`')  //切换密码
        {
            AO=' ';
            flag=1;
            m=0;//清空数组下标，不影响账号密码输入
        }
        else if(AO == '-')  
        {
            AO=' ';
            if(m > 0)//判断是否有内容
                m--;
                if(flag == 0)
                {
                    account_buf[m]=AO;  
                    account_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //再次调用，实现清除数据功能，刷新文本框
                }
                else
                {
                    password_buf[m]=AO;
                    hidepassword_buf[m]=AO; 
                    password_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //再次调用，实现清除数据功能，刷新文本框
                }
        }
        else if(AO == '=' && flag == 1)//确认密码已输入,登入
        {
            printf("账号:%s\n密码:%s\n",account_buf,password_buf);
            break;
        }

        //判断账号密码长度是否超过界限
        else if(m < 128 && AO != '=')    
        {
            if(flag == 0)
            {
                account_buf[m]=AO;
                m++;
            }
            else
            {
                password_buf[m]=AO;
                hidepassword_buf[m]='*'; //隐藏密码
                m++;
            }
        }

        //显示修改
        if(flag == 0)
        {
            fontPrint(f,account_textbox,5,4,account_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,81,200,account_textbox);
        }
        else
        {
            fontPrint(f,password_textbox,5,4,hidepassword_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,81,245,password_textbox);
        }
    }
    destroyBitmap(account_textbox);
    destroyBitmap(password_textbox);
    close(lcd_fd);
    fontUnload(f);
    munmap(p,800*480*4);
    return 0;
}

//未点开游戏文本框提示功能实现
int no_game_textbox_fun()
{
    int lcd_fd = open("/dev/fb0", O_RDWR);
    int *p = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
    //初始化字库 f:字库句柄 "simkai.ttf":字库名字
    font *f = fontLoad("simkai.ttf");
    //设置字体大小 32:字体大小
    fontSetSize(f,32);
    //设置文本框大小 300*40：表示文本框大小 4：表示色素    A R G B
    //未点开游戏文本框 no_game_textbox：表示未点开游戏文本框
    //no_game_buf：表示未点开游戏文本框显示的字符串
    bitmap *no_game_textbox = createBitmapWithInit(400, 40, 4,getColor(0,255,255,255)); //白色
    char no_game_buf[128]="未点击到游戏，请再次点开。";
    fontPrint(f,no_game_textbox,5,4,no_game_buf,getColor(0,0,0,0),0); 
    show_font_to_lcd(p,200,330,no_game_textbox);
    close(lcd_fd);
    fontUnload(f);
    munmap(p,800*480*4);
    destroyBitmap(no_game_textbox);
}

//桌面-游戏功能实现
int home_fun()  //主界面功能实现
{
    show_bmp_to_lcd("home.bmp");    //显示主界面
    while(1)    //是否点击游戏
    {
        input_fun();   //调用触摸屏功能实现，获取坐标数据
        if (input_x >= 333 && input_x <= 390 && input_y >= 210 && input_y <= 275)  //正确触碰到游戏打开范围
        {
            int game_fun()  //游戏功能实现
            {
                int game_initialization()    //游戏初始化功能实现
                {
                    show_bmp_to_lcd("logo.bmp");   //显示游戏logo界面
                    usleep(3000000);    //延时3秒
                    show_bmp_to_lcd("anti_addiction.bmp");    //显示防沉迷提示界面
                    usleep(3000000);    //延时3秒

                    int input_account_fun()  //账号密码输入功能实现
                    {
                        show_bmp_to_lcd("login_1.bmp");    //显示登陆界面
                        input_fun();   //调用触摸屏功能实现，获取坐标数据
                        while(1)    //是否点击账号密码输入界面，是则输入账号和密码；若点击退出按钮，则返回主界面，若点击除上述外的区域，保持登陆界面不变
                        {
                            if (input_x >= 318 && input_x <= 496 && input_y >= 217 && input_y <= 392)    //点击登录按钮                        {
                                {
                                    show_bmp_to_lcd("login_2.bmp");    //显示账号密码输入界面
                                    account_password_textbox_fun();   //调用账号密码文本框功能实现
                                    break;    //跳出输入账号密码界面循环
                                }
                        
                            if (input_x >= 729 && input_x <= 798 && input_y >= 409 && input_y <= 469)    //点击退出按钮
                            {
                                home_fun();    //返回主界面
                                break;    //跳出输入账号密码界面循环
                            }   
                            else    //点击其他区域，保持登陆界面不变
                            {  
                                show_bmp_to_lcd("login_1.bmp");    //显示登陆界面
                                input_fun();   //重新监听，直到点击登录按钮或退出按钮
                            }  
                        }  
                    } 
                    input_account_fun();   //调用账号密码输入功能实现
                }
                game_initialization();   //调用游戏初始化功能实现
            }
            game_fun();   //调用游戏功能实现
            break;   //跳出主界面循环
        }
        else    //未点击游戏打开范围
        {
            show_bmp_to_lcd("home.bmp");    //显示主界面
            no_game_textbox_fun();   //调用未点开游戏文本框提示功能实现
            usleep(2000000);    //延时2秒,缓冲
            show_bmp_to_lcd("home.bmp");    //显示主界面
            input_fun();   //重新监听，直至点击游戏打开范围
        }
    }
}

//三、主函数
int main(int argc, char const *argv[])
{
    home_fun();   //主界面功能实现
    return 0;
}


