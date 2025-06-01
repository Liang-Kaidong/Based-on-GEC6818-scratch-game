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

构建版本：V3.3
构建日期：2024-12-24 11:30:07 UTC+8
维护者：KD

更新日志：
1.修复已知问题，提高系统稳定性。
2.修复虚拟键盘在某些场合无响应的问题。
3.修复虚拟键盘位置异常偏移的问题。
4.修复游戏界面退出提示，位置坐标异常无响应的问题。
5.修复游戏界面退出提示，点击位置异常偏移的问题。
6.优化代码结构，提高代码可读性。
7.优化注释显示方式，提高代码可读性。
8.优化虚拟键盘功能的显示排版。
9.新增虚拟键盘功能，可以输入账号密码。
10.游戏登录界面退出提示功能实现
11.删除通过外设输入账号密码的功能。(可自主启用)

温馨提示：为实现良好的游戏体验，本人在主界面、游戏界面等
         加入了许多嵌套循环代码，在未熟知代码的前提下，
         贸然修改循环结构将导致程序的异常崩溃，
         因此，在修改代码结构时，应当充分考虑代码的逻辑，
         确保修改后的代码能够正常运行。
*/

//全局变量定义
int app_click = 0;  //判断是否正确触碰到游戏 0：点击到了游戏 1：未点击游戏+
int x,y;         //触摸屏的坐标
int input_x, input_y; // 点击屏幕的坐标

//图片显示功能实现
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


/*
//账号和密码文本框功能实现(使用外设实现)  当前已弃用
int account_number_buf_password_number_buf_textbox_fun()
{
    //申请内存映射
    int lcd_fd = open("/dev/fb0", O_RDWR);
    int *p = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
    //初始化字库 f:字库句柄 "simkai.ttf":字库名字
    font *f = fontLoad("simkai.ttf");
    //设置字体大小 30:字体大小
    fontSetSize(f,30);
    //设置文本框大小 300*40：表示文本框大小 4：表示色素    A R G B
    //账号密码文本框 account_number_buf_textbox：表示账号文本框 password_number_buf_textbox：表示密码文本框
    //account_number_buf_buf：表示账号文本框显示的字符串
    //password_number_buf_buf：表示密码文本框显示的字符串
    //hide_password_number_buf_buf：表示密码文本框显示的字符串，隐藏密码
    //判断变量：m
    char account_number_buf_buf[128]={0};
    char password_number_buf_buf[128]={0};
    char hidepassword_number_buf_buf[128]={0}; //隐藏密码
    bitmap *account_number_buf_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //白色
    bitmap *password_number_buf_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //白色
    int m=0;    //判断变量m
    int flag=0; //flag=0表示账号输入，flag=1表示密码输入   
    //将字体输入到文本框内 account_number_buf_textbox:表示文本框 password_number_buf_textbox:表示密码文本框 0,0：表示字体显示的位置
    //fontPrint(f,account_number_buf_textbox,0,0,account_number_buf_buf,getColor(0,0,0,0),0); 
    //fontPrint(f,password_number_buf_textbox,0,0,password_number_buf_buf,getColor(0,0,0,0),0); 
    //将文本框显示到LCD上 81,200：文本框在LCD上显示的起始位置
    show_font_to_lcd(p,81,200,account_number_buf_textbox);
    show_font_to_lcd(p,81,245,password_number_buf_textbox);


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
                account_number_buf_buf[m]=AO;
                m++;
            }
            else
            {
                password_number_buf_buf[m]=AO;
                m++;
            }
        }

        //显示修改
        if(flag==0)
        {
            fontPrint(f,account_number_buf_textbox,5,4,account_number_buf_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,150,150,account_number_buf_textbox);
        }
        else
        {
            fontPrint(f,password_number_buf_textbox,5,4,password_number_buf_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,150,250,password_number_buf_textbox);
        }
    }

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
                    account_number_buf_buf[m]=AO;  
                    account_number_buf_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //再次调用，实现清除数据功能，刷新文本框
                }
                else
                {
                    password_number_buf_buf[m]=AO;
                    hidepassword_number_buf_buf[m]=AO; 
                    password_number_buf_textbox = createBitmapWithInit(300, 40, 4,getColor(0,255,255,255)); //再次调用，实现清除数据功能，刷新文本框
                }
        }
        else if(AO == '=' && flag == 1)//确认密码已输入,登入
        {
            printf("账号:%s\n密码:%s\n",account_number_buf_buf,password_number_buf_buf);
            break;
        }

        //判断账号密码长度是否超过界限
        else if(m < 128 && AO != '=')    
        {
            if(flag == 0)
            {
                account_number_buf_buf[m]=AO;
                m++;
            }
            else
            {
                password_number_buf_buf[m]=AO;
                hidepassword_number_buf_buf[m]='*'; //隐藏密码
                m++;
            }
        }

        //显示修改
        if(flag == 0)
        {
            fontPrint(f,account_number_buf_textbox,5,4,account_number_buf_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,81,200,account_number_buf_textbox);
        }
        else
        {
            fontPrint(f,password_number_buf_textbox,5,4,hidepassword_number_buf_buf,getColor(0,0,0,0),0); 
            show_font_to_lcd(p,81,245,password_number_buf_textbox);
        }
    }
    destroyBitmap(account_number_buf_textbox);
    destroyBitmap(password_number_buf_textbox);
    close(lcd_fd);
    fontUnload(f);
    munmap(p,800*480*4);
    return 0;
}
*/

//未点开游戏文本框提示功能实现
int no_game_textbox_fun()
{
    int lcd_fd = open("/dev/fb0", O_RDWR);
    int *p = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);
    //初始化字库 f:字库句柄 "simkai.ttf":字库名字
    font *f = fontLoad("simkai.ttf");
    //设置字体大小 30:字体大小
    fontSetSize(f,30);
    //设置文本框大小 400*40：表示文本框大小 4：表示色素    A R G B
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
    return 0;
}

//虚拟键盘功能实现
int input_fd;   //触摸屏文件描述符
struct input_event input_buf;   //存储触摸屏数据
int input_event(int *input_x, int *input_y)   //获取点击屏幕坐标的事件
{
    read(input_fd, &input_buf, sizeof(input_buf));
    if (input_buf.type == 3 && input_buf.code == 0)
        *input_x = input_buf.value * 800 / 1022;
    if (input_buf.type == 3 && input_buf.code == 1)
        *input_y = input_buf.value * 480 / 598;
}
int virtual_keyboard_fun() 
{
    char account_buf[4] = "";     //用于存储账号
    char password_buf[4] = "";    //用于存储密码
    char hide_password_buf[21] = ""; //用于存储隐藏密码
    int lcd_fd = open("/dev/fb0", O_RDWR);
    if (lcd_fd == -1) {
        printf("打开液晶屏失败.\n");
        return -1;
    }
    input_fd = open("/dev/input/event0", O_RDWR);
    if (input_fd == -1) {
        printf("打开触摸屏失败.\n");
        return -2;
    }

    
    int *p = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,lcd_fd,0);     //内存映射
    font *f = fontLoad("simkai.ttf");    //初始化字库 f:字库句柄 "simkai.ttf":字库名字
    fontSetSize(f,30);  //设置字体大小 30:字体大小

    //设置文本框大小 80*50:表示文本框大小 4：表示色素     A  R  G  B  （0123456789，+，-，确认，删除，清零）
    bitmap *number_textbox_1 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));  //数字1的文本框
    bitmap *number_textbox_2 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));  //数字2的文本框
    bitmap *number_textbox_3 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));  //数字3的文本框
    bitmap *number_textbox_4 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));   //数字4的文本框
    bitmap *number_textbox_5 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));   //数字5的文本框
    bitmap *number_textbox_6 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));   //数字6的文本框
    bitmap *number_textbox_7 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));   //数字7的文本框
    bitmap *number_textbox_8 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));   //数字8的文本框
    bitmap *number_textbox_9 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));   //数字9的文本框
    bitmap *number_textbox_0 = createBitmapWithInit(80,80,4,getColor(0,255,255,255));  //数字0的文本框
    bitmap *delete_textbox = createBitmapWithInit(80,80,4,getColor(0,255,255,255));   //删除的文本框
    bitmap *clean_textbox = createBitmapWithInit(80,80,4,getColor(0,255,255,255));     //清空的文本框
    bitmap *sum_textbox = createBitmapWithInit(80,80,4,getColor(0,255,255,255));      //加号的文本框
    bitmap *sub_textbox = createBitmapWithInit(80,80,4,getColor(0,255,255,255));      //减号的文本框
    bitmap *O_textbox = createBitmapWithInit(80,80,4,getColor(0,255,255,255));         //"O"的文本框
    bitmap *K_textbox = createBitmapWithInit(80,80,4,getColor(0,255,255,255));         //"K"的文本框
    bitmap *account_textbox = createBitmapWithInit(280,40,4,getColor(0,255,255,255));   //账号的文本框
    bitmap *password_text = createBitmapWithInit(280,40,4,getColor(0,255,255,255));     //密码的文本框
    bitmap *hide_password_text = createBitmapWithInit(280,40,4,getColor(0,255,255,255)); //隐藏密码的文本框
    
    //字符数组
    char number_buf_1[21]="1";
    char number_buf_2[21]="2";
    char number_buf_3[21]="3";
    char number_buf_4[21]="4";
    char number_buf_5[21]="5";
    char number_buf_6[21]="6";
    char number_buf_7[21]="7";
    char number_buf_8[21]="8";
    char number_buf_9[21]="9";
    char number_buf_0[21]="0";
    char delete_buf[21]="删除";
    char clean_buf[21]="全清";
    char sum_buf[21]="爱";
    char sub_buf[21]="你";
    char O_buf[21]="确";
    char K_buf[21]="认";

    //将字体输入到文本框内 number_textbox:表示数字的文本框 0,0：表示字体显示的起始位置
    fontPrint(f,number_textbox_1,30,25,number_buf_1,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_2,30,25,number_buf_2,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_3,30,25,number_buf_3,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_4,30,25,number_buf_4,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_5,30,25,number_buf_5,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_6,30,25,number_buf_6,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_7,30,25,number_buf_7,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_8,30,25,number_buf_8,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_9,30,25,number_buf_9,getColor(0,0,0,0),0);
    fontPrint(f,number_textbox_0,33,25,number_buf_0,getColor(0,0,0,0),0);
    fontPrint(f,delete_textbox,0,25,delete_buf,getColor(0,0,0,0),0);
    fontPrint(f,clean_textbox,0,25,clean_buf,getColor(0,0,0,0),0);
    fontPrint(f,sum_textbox,25,25,sum_buf,getColor(0,0,0,0),0);
    fontPrint(f,sub_textbox,22,25,sub_buf,getColor(0,0,0,0),0);
    fontPrint(f,O_textbox,15,25,O_buf,getColor(0,0,0,0),0);
    fontPrint(f,K_textbox,20,25,K_buf,getColor(0,0,0,0),0);
    fontPrint(f,account_textbox,0,7,account_buf,getColor(0,0,0,0),0);
    fontPrint(f,password_text,0,7,hide_password_buf,getColor(0,0,0,0),0);

    //将文本框显示到LCD上 400,150：文本框在LCD上显示的起始位置
    //第一行(1,2,3,删除)
    show_font_to_lcd(p,400,150,number_textbox_1);
    show_font_to_lcd(p,480,150,number_textbox_2);
    show_font_to_lcd(p,560,150,number_textbox_3);
    show_font_to_lcd(p,640,150,delete_textbox);
    //第二行(30,25,6,全清)
    show_font_to_lcd(p,400,230,number_textbox_4);
    show_font_to_lcd(p,480,230,number_textbox_5);
    show_font_to_lcd(p,560,230,number_textbox_6);
    show_font_to_lcd(p,640,230,clean_textbox);
    //第三行(7,8,9,确)
    show_font_to_lcd(p,400,310,number_textbox_7);
    show_font_to_lcd(p,480,310,number_textbox_8);
    show_font_to_lcd(p,560,310,number_textbox_9);
    show_font_to_lcd(p,640,310,O_textbox);
    //第四行(加,0,减,认)
    show_font_to_lcd(p,400,390,sum_textbox);
    show_font_to_lcd(p,480,390,number_textbox_0);
    show_font_to_lcd(p,560,390,sub_textbox);
    show_font_to_lcd(p,640,390,K_textbox);
    //账号密码的文本框显示
    show_font_to_lcd(p,81,200,account_textbox);
    show_font_to_lcd(p,81,250,password_text);

    int out_put_state = 0; // 0: 输入账号, 1: 输入密码
    int touch_state = 0; // 触摸状态 0: 未触摸, 1: 已触摸

    while(1)
    {
        input_event(&input_x, &input_y);   // 只有在触摸按键时才进行处理
        if (input_buf.type == 1 && input_buf.code == 330 && input_buf.value == 1)   // input_buf.value == 1 表示触摸按下
        { 
            // 键盘触摸判断区域
            if (input_x >= 506 && input_x <= 558 && input_y >= 409 && input_y <= 469)    //数字0触摸区域
            {
                if (out_put_state == 0)     // 输入账号
                {
                    if (touch_state == 0)   //确保只处理一次触摸
                    {  
                        strcat(account_buf, "0");   //账号输入0 (拼接之前的数据)
                        touch_state = 1;  // 更新触摸状态
                    }
                }
                else    // 输入密码
                {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "0");   //密码输入0 (拼接之前的数据)
                        strcat(hide_password_buf, "*");    //隐藏密码输入* (拼接之前的数据)
                        touch_state = 1;
                    }
                }
            }
            else if (input_x >= 411 && input_x <= 472 && input_y >= 162 && input_y <= 215) 
            {
                if (out_put_state == 0)    // 输入账号
                {
                    if (touch_state == 0) 
                    {  
                        strcat(account_buf, "1");   //账号输入1(拼接之前的数据)
                        touch_state = 1;  // 更新触摸状态
                    }
                }
                else 
                {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "1");   //密码输入1 (拼接之前的数据)
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 505 && input_x <= 547 && input_y >= 162 && input_y <= 215) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "2");
                        touch_state = 1;
                    }
                } 
                else 
                {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "2");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 580 && input_x <= 631 && input_y >= 162 && input_y <= 215) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "3");
                        touch_state = 1;
                    }
                } 
                else {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "3");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 653 && input_x <= 715 && input_y >= 162 && input_y <= 215) 
            {
                if (out_put_state == 0 && strlen(account_buf) > 0) 
                {
                    account_buf[strlen(account_buf) - 1] = '\0';  // 删除账号的最后一个字符
                    touch_state = 1;
                } 
                else if (out_put_state == 1 && strlen(password_buf) > 0) 
                {
                    password_buf[strlen(password_buf) - 1] = '\0';  // 删除密码的最后一个字符
                    hide_password_buf[strlen(hide_password_buf) - 1] = '\0';     // 删除隐藏密码的最后一个字符
                    touch_state = 1;
                }
            } 
            else if (input_x >= 411 && input_x <= 472 && input_y >= 252 && input_y <= 293) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "4");
                        touch_state = 1;
                    }
                } 
                else {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "4");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 505 && input_x <= 547 && input_y >= 255 && input_y <= 293) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "5");
                        touch_state = 1;
                    }
                } 
                else {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "5");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 580 && input_x <= 631 && input_y >= 255 && input_y <= 293) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "6");
                        touch_state = 1;
                    }
                } 
                else {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "6");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 653 && input_x <= 715 && input_y >= 255 && input_y <= 293) 
            {
                if (out_put_state == 0) 
                {
                    memset(account_buf, 0, sizeof(account_buf));  // 清空账号
                    touch_state = 1;
                } 
                else 
                {
                    memset(password_buf, 0, sizeof(password_buf));  // 清空密码
                    memset(hide_password_buf, 0, sizeof(hide_password_buf));     // 清空隐藏密码
                    touch_state = 1;
                }
            } 
            else if (input_x >= 411 && input_x <= 472 && input_y >= 315 && input_y <= 371) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "7");
                        touch_state = 1;
                    }
                } 
                else 
                {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "7");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 505 && input_x <= 547 && input_y >=315 && input_y <=371) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "8");
                        touch_state = 1;
                    }
                } 
                else {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "8");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 580 && input_x <= 631 && input_y >=315 && input_y <=371) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        strcat(account_buf, "9");
                        touch_state = 1;
                    }
                } 
                else 
                {
                    if (touch_state == 0) 
                    {
                        strcat(password_buf, "9");
                        strcat(hide_password_buf, "*");
                        touch_state = 1;
                    }
                }
            } 
            else if (input_x >= 653 && input_x <= 715 && input_y >=315 && input_y <=475) 
            {
                if (out_put_state == 0) 
                {
                    if (touch_state == 0) 
                    {
                        //strcat(account_buf, "0");     //确认键设定为无效
                        touch_state = 1;
                        break;    //跳出输入账号密码界面循环
                    }
                } 
                else {
                    if (touch_state == 0) 
                    {
                        //strcat(password_buf, "0");     //确认键设定为无效
                        touch_state = 1;
                        break;    //跳出输入账号密码界面循环
                    }
                }
            } 
            else if (input_x >= 81 && input_x <= 363 && input_y >= 200 && input_y <= 249) 
            {
                // 点击到输入账号的区域
                out_put_state = 0;
                touch_state = 1;
            } 
            else if (input_x >= 81 && input_x <= 363 && input_y >= 250 && input_y <= 290) 
            {
                // 输入密码
                out_put_state = 1;
                touch_state = 1;
            }
        }

        // 如果触摸事件结束（input_buf.value == -1），则重置触摸状态
        if (input_buf.type != 1 || input_buf.code != 330 || input_buf.value != 1) 
        {
            touch_state = 0;  // 触摸结束，允许下一次触摸识别
            //printf("触摸坐标: (%d, %d)\n", input_x, input_y);
        }

        //检测是否超过账号和密码长度限制
        int account_len = strlen(account_buf);
        int password_len = strlen(password_buf);
        int hide_password_len = strlen(hide_password_buf);
        if (account_len > 3 || password_len > 3 || hide_password_len > 3) 
        {
            printf("账号或密码长度超过限制\n");
            break;    //跳出输入账号密码界面循环
        }

        //刷新文本框，防止文字重叠
        bitmap *account_textbox = createBitmapWithInit(280,40,4,getColor(0,255,255,255));
        bitmap *password_text = createBitmapWithInit(280,40,4,getColor(0,255,255,255));
        bitmap *hide_password_text = createBitmapWithInit(280,40,4,getColor(0,255,255,255));
        fontPrint(f,account_textbox,0,7,account_buf,getColor(0,0,0,0),0);
        fontPrint(f,password_text,0,7,hide_password_buf,getColor(0,0,0,0),0);
        show_font_to_lcd(p,81,200,account_textbox);
        show_font_to_lcd(p,81,250,password_text);
    }

    //debug账号密码输入功能测试
    //printf("账号: %s\n", account_buf);
    //printf("密码: %s\n", password_buf);
    //printf("隐藏密码: %s\n", hide_password_buf);

    //关闭文件
    close(lcd_fd);
    close(input_fd);
    fontUnload(f);
    destroyBitmap(account_textbox);
    destroyBitmap(password_text);
    munmap(p, 800 * 480 * 4);
    return 0;
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

                    int input_account_number_buf_fun()  //账号密码输入功能实现
                    {
                        show_bmp_to_lcd("login_1.bmp");    //显示登陆界面
                        input_fun();   //调用触摸屏功能实现，获取坐标数据
                        while(2)    //是否点击账号密码输入界面，是则输入账号和密码；若点击退出按钮，则返回主界面，若点击除上述外的区域，保持登陆界面不变
                        {
                            if (input_x >= 318 && input_x <= 496 && input_y >= 217 && input_y <= 392)    //点击登录按钮                        
                                {
                                    show_bmp_to_lcd("login_2.bmp");    //显示账号密码输入界面
                                    //account_number_buf_password_number_buf_textbox_fun();   //调用使用外设填入账号密码文本框功能实现(已弃用，可自主开启)
                                    virtual_keyboard_fun();   //调用虚拟键盘功能实现
                                    break;    //跳出输入账号密码界面循环
                                }
                            if (input_x >= 729 && input_x <= 798 && input_y >= 409 && input_y <= 476)    //点击游戏退出按钮
                                {   
                                    show_bmp_to_lcd("login_exit.bmp");    //显示游戏退出确认界面
                                    while(3)
                                    {
                                        input_fun();   //调用触摸屏功能实现，获取坐标数据
                                        if(input_x > 243 && input_x < 390 && input_y > 284 && input_y < 360)    //确定退出
                                            {
                                                home_fun();   //返回主界面
                                            }
                                        if(input_x > 400 && input_x < 547 && input_y > 284 && input_y < 360)    //取消退出
                                            {
                                                input_account_number_buf_fun();    //返回登录界面
                                            }
                                        else    
                                            {
                                                break;    //重新回到游戏登录界面
                                            }
                                    }
                                }   
                            else    //点击其他区域，保持登陆界面不变
                            {  
                                show_bmp_to_lcd("login_1.bmp");    //显示登陆界面
                                input_fun();   //重新监听，直到点击登录按钮或退出按钮
                            }  
                        }  
                        return 0;
                    } 
                    input_account_number_buf_fun();   //调用账号密码输入功能实现
                    return 0;
                }
                game_initialization();   //调用游戏初始化功能实现
                return 0;
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
    return 0;
}

//主函数
int main(int argc, char const *argv[])
{
    home_fun();   //主界面功能实现
    return 0;
}


