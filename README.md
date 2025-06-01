# 基于GEC6818的刮刮乐游戏-粤嵌实训项目
> **项目要求：**
1. 登录注册，注册：实现账号永久保持，将数据保存在文档内 登录：实现查询账号是否被冻结
2. 密码找回（电话找回，发送验证码实现找回），冻结解封
3. 刮刮乐：中奖概率，实现再来一次，充值界面（余额显示，以及扣钱机制）中奖奖励
4. UI界面设置好看
5. 全程使用触摸屏实现，登录注册，密码找回需要用字库，密码不能显示
6. ~~不能出现bug~~（放过我吧）

**该项目基于粤嵌GEC6818开发板开发的。项目包括了：
①全程使用触摸屏进行操作，包含虚拟键盘的操作；
②指定位置显示指定大小的BMP图片，从BMP图片当中读取指定信息并显示在LCD屏指定位置；
③结构体、数组、Strcmp、Sprintf的使用，用于实现账号的注册、登录、找回，实现本地账号文档的建立、修改与删除（结构体自己弄吧）；
④使用粤嵌公司提供的Libfont.a字库，实现文字的显示，利用Bitmap实现类文本框；
⑤使用随机数实现端对端双向数据校验，提供游戏账户使用验证码的方式安全找回
实现游戏中，刮刮乐图层的刮去与重覆盖。**

**使用的交叉编译链为arm-linux-gcc,目录中的文件仅适用于GEC6818开发板！由于启动该项目时，并没有严格按照注释手册规范注释，所以会看得比较难受。另外，由于该项目有部分功能还未能实现，就算我想到了却没有那个板子，所以的话得后续的开发者自行解决。**

## 0.目录索引
| 目录索引 | 简要介绍 |
| --- | --- |
| Histroy_build | 有关游戏的历史构建版本 |
| lesson | 课程教材文档 |
| pic | 游戏所需的bmp图片 |
| font.h | 字库头文件（粤嵌提供） |
| libfont.a | 字库文件（粤嵌提供） |
| p | 可执行程序 |
| project_V4.0.c | 游戏文件 |
| Update_logs.txt | 更新日志 |
| 项目要求.txt | 项目要求 |
| README.md | 说明文件 |

## 1.项目验收

一、程序运行主界面

![屏幕截图 2025-06-01 202842](https://github.com/user-attachments/assets/faf68eeb-b619-452b-afaa-5403cdbfddc7)

二、游戏登陆界面

![屏幕截图 2025-06-01 202929](https://github.com/user-attachments/assets/1dc8c07f-60ce-439e-ba00-2207fb76361a)

三、账号或密码错误

![屏幕截图 2025-06-01 203021](https://github.com/user-attachments/assets/8258f0ec-32f3-4cd4-be7d-71b1d3ee6845)

四、账号冻结界面

![屏幕截图 2025-06-01 203043](https://github.com/user-attachments/assets/19c0e1fe-b40b-46ab-a525-850bc6853b29)

五、密码找回界面

![屏幕截图 2025-06-01 203107](https://github.com/user-attachments/assets/cd2f50fd-013b-474b-8d2a-2a862237fcb2)

![屏幕截图 2025-06-01 203125](https://github.com/user-attachments/assets/0685a567-8ecd-4d05-b7b5-cbcf56a48728)

六、账号注册界面

![屏幕截图 2025-06-01 203001](https://github.com/user-attachments/assets/a3980a92-815f-4b25-aefa-03ad47fdc661)

七、登录成功后界面

![屏幕截图 2025-06-01 203144](https://github.com/user-attachments/assets/8b4dcfa4-ef53-4562-a1d4-997cd7c937eb)

八、图层刮涂

![屏幕截图 2025-06-01 203213](https://github.com/user-attachments/assets/878b648e-5f81-436c-b16c-c26234fe2412)

## 2.知识储备
> 由于每一个作业对应的教材太过冗长，这里不便展示，如有需要请自行下载讲义的文件。

由于讲义文件太过久远，不方便进行大规模的改动。如果和你所学的知识有所冲突，请以你的为准！

## 3.环境配置
> 环境搭建方法详情请预览->讲义->环境配置->[环境安装（WSL版本）.docx](https://github.com/user-attachments/files/20539316/WSL.docx)

```
通过网盘分享的文件：WSL.zip
链接: https://pan.baidu.com/s/1uVpFF8f198GFeehKkCPRtQ 提取码: rmag
```
资料请下载 WSL.zip

## 4.有关字库的使用教程（非自建字库）
### 一、初始化字库
font *fontLoad(char *fontPath);

参数一：DroidSansFallback.ttf 字库的路径

返回值：操作字库的句柄

```
    // 打开字体
    font *f = fontLoad("simkai.ttf");
```

### 二、设置字体的大小
void fontSetSize(font *f, s32 pixels)

参数一：操作的字库

参数二：字体的大小

```
    fontSetSize(f, 32);//32:长和宽32像素点
```

### 三、设置字体输出框的大小
bitmap *createBitmapWithInit(u32 width, u32 height, u32 byteperpixel，int color);

参数一：输出框的宽 0~800

参数二：输出框的高 0~480

参数三：当前屏幕的色素    例如：32位-》4

参数四：输出框的颜色

```
    bitmap *bm = createBitmapWithInit(200, 100, 4,getColor(0,255,255,255));  //白色   
```

### 四、把字体输出到输出框中
void fontPrint(font *f, bitmap *screen, s32 x, s32 y, char *text, color c, s32 maxWidth)

参数一：操作的字库

参数二：输出框

参数三：字体的位置    X轴

参数四：字体的位置    Y轴

参数五：字体文本（输出内容）

参数六：字体颜色

参数七：一行显示字体的最大宽度    （例如：字体大小为32，假设设置为64则一行显示两个字体，就换行显示）

默认为： 0

```
    fontPrint(f,bm,0,0,buf,getColor(0,0,0,0),0);
```

### 五、把输出框的所有信息显示到LCD屏幕中
void show_font_to_lcd(unsigned int *p,int px,int py,bitmap *bm)

参数一：LCD映射后的首地址(内存映射)

参数二：文本框的位置 X轴 输出框在液晶屏上的位置

参数三：文本框的位置 Y轴

参数四：设置好的字体输出框

```
    //把字体框输出到LCD屏幕上
    show_font_to_lcd(p,200,200,bm);
```

### 六、销毁所有初始化的东西

```
    // 关闭字体
    void fontUnload(font *f);
    fontUnload(f);
    // 关闭输出框
    void destroyBitmap(bitmap *bm)
    destroyBitmap(bm);
    注意：颜色的设置要用  getColor(A,B,G,R)
```

### 七、编译

```
    arm-linux-gcc  zi.c -o zi -L./ -lfont -lm
```

## 5.字符串处理函数
### 字符串处理函数
#### 一、函数 strstr（严格匹配大小写）
注意：在使用忽略大小写操作的函数时需要在头文件前面添加一个宏的定义 

![图片1](https://github.com/user-attachments/assets/d979a073-04eb-4416-ab5c-9101de6f9919)

```
    #define _GNU_SOURCE
```

```
     示例：
        char *s = "abcd.txt";
        char *p = strstr(s, ".wps");
        
        if(p == NULL)
            printf("文件[%s]不是WPS文件\n", s);
        else
            printf("文件[%s]是WPS文件\n", s);
    
        #define _GNU_SOURCE
        #include <stdio.h>
        #include <string.h>
    
        int main(int argc, char const *argv[])
        {
            char *p1 = "Hello Even GZ2315 Even";
            char *p2 = "even";
        
            printf("p1:%p\n" , p1);
            printf("p2:%p\n" , p2);
        
            // char *ptr = strstr( p1 , p2 );  // 严格匹配大小写
            char *ptr = strcasestr( p1 , p2 );  // 匹配的过程中忽略大小写 注意在头文件前加上#define _GNU_SOURCE 
            if (NULL == ptr)
            {
                printf("找不到目标子串..\n");
            }
            else{
                printf("找到子串，他的入口地址是：%p:%s\n" , ptr , ptr );
            }
            return 0;
        }    
```

#### 二、函数strlen
注意：该函数是一个strxxxx类的函数，因此他的结束条件是 '\0' ， 所以在使用该函数的时候他计算的长度只到第一个出现的'\0'为止，而且这个结束符'\0'不在计算的范围内；

![图片2](https://github.com/user-attachments/assets/f227629d-7c7d-45bd-9992-8510b2e9c956)

```
     示例：
        char *s = "www.\0yueqian.com.cn";
        printf("粤嵌官网地址的长度是：%d\n", strlen(s));
```

#### 三、函数strtok
 注意：该函数会将改变原始字符串 str（因此str必须是指向一个可读可写的入口地址），使其所包含的所有分隔符变成结束标记‘\0’。由于该函数需要更改字符串 str，因此 str 指向的内存必须是可写的。首次调用时 str 指向原始字符串，此后每次调用 str 用 NULL 代替。分隔符组合 delim 可以由多个字符组成，在分割的时候这多个字符会被单独匹配，匹配成功则把str中对应的字符替换为 '\0'

![图片3](https://github.com/user-attachments/assets/b46d1e0b-80f9-4234-a32e-57fd08671aa1)

```
        char buf[10]="123,456";
        char buf1[3]={0};
        char seqs[] = ",";  //分割的字符
        char *tmp = strtok(buf,seqs);//123
        strcpy(buf1,tmp);
    
        tmp = strtok(NULL,seqs);//456 继续往下分割
        strcpy(buf1,tmp);
```

#### 四、函数strcpy与strncpy
 注意：这两个函数的功能，都是将 src 中的字符串，复制到 dest 中。strcpy() 没有边界控制，因此可能会由于 src 的过长而导致内存溢出。strncpy() 有边界控制，最多复制 n+1 个字符（其中最后一个是 ‘\0’ ）到 dest 中。
 
![图片4](https://github.com/user-attachments/assets/78ad2d14-da68-49bf-8946-95594b4af2b2)

#### 五、函数strcmp与strncmp
 注意：比较字符串大小，实际上比较的是字符的 ASCII码值的大小。从左到右逐个比较两个字符串的每一个字符，当能“决出胜负”时立刻停止比较。从左到右逐个进行字符之间的减法运算，不等与0则表示不相等，因此相等返回 0  否则返回差值，所有的以str开头的函数都是在遇到第一个'\0'结束符的时候停止运行返回。如果函数中有 n 则有两种结束的情况 '\0' + 到达用户指定 N字节

![图片5](https://github.com/user-attachments/assets/e0fe80cc-2c0f-46e4-8480-26819f745a07)

```
    示例：
    printf("%d\n", strcmp("abc", "abc")); // 输出0，两个字符串相等
    printf("%d\n", strcmp("abc", "aBc")); // 输出32 差值，"abc" 大于 "aBc"
    printf("%d\n", strcmp("aBc", "abc")); // 输出-32差值
    printf("%d\n" , strncmp(p1 , p2 , 4 )); // 比较两个字符串的前 4字节
```


## 6.致谢
感谢广州粤嵌公司进行教学指导，部分文件由粤嵌公司提供。项目仅供学习与参考，如涉及文件侵权请及时与我联系进行删除，禁止一切售卖行为与违法行为！
