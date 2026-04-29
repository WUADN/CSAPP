# csapp-labs

Book: Computer Systems: A Programmer's Perspective, 3/E (CS:APP3e)

Labs for self-study, downloaded from: [http://csapp.cs.cmu.edu/3e/labs.html](http://csapp.cs.cmu.edu/3e/labs.html)

简要说明实验完成: 实验内容随机,所以不存在照抄说法(当然思路基本类似)


## attack lab
这个部分实验的基本原理是: stack位置不变,通过过量写入buf篡改返回地址,让其指向栈的某个位置,从而执行注入栈的代码

### part1: level3
栈会被覆盖,通过gdb检查80byte不变字节,在那里保存cookie.
还有"cookie字符串"是指cookie字面量的ascii形式,我开始以为是填充'\0'就可以了:(


