# csapp-labs

Book: Computer Systems: A Programmer's Perspective, 3/E (CS:APP3e)

Labs for self-study, downloaded from: [http://csapp.cs.cmu.edu/3e/labs.html](http://csapp.cs.cmu.edu/3e/labs.html)

简要说明实验完成: 实验内容随机,所以不存在照抄说法(当然思路基本类似)


## attack lab
这个部分实验的基本原理是: stack位置不变,通过过量写入buf篡改返回地址,让其指向栈的某个位置,从而执行注入栈的代码

### part1: level3
栈会被覆盖,通过gdb检查80byte不变字节,在那里保存cookie.
还有"cookie字符串"是指cookie字面量的ascii形式,我开始以为是填充'\0'就可以了:(


### part2: level4
注意地址为8字节,将cookie放在第一个gadget上面.


### part2: level5
没有完成,大致思路应该是使用某个gadget移动rsp到cookie的地址(cookie放到合适位置),然后push $rsp到参数中,因为栈是动态变化的.感觉要花很多时间:)


## cache lab
### partA
结构体设计以及cache访问算法是主要部分，获取参数-> 扫描文件 -> 访问cache。可以将部分参数放到main
函数外面，避免函数设计参数过多(hit, miss, evicton..)
