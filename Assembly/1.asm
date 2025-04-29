data segment
s db 100 dup(0)
t db 100 dup(0)
data ends
;以上是标准的数组定义 应用了dup重复100次
code segment
assume cs:code , ds:data;这段是从课本上抄来的
main:
	mov si,00h;si是s数组的下标，先初始化为0，准备进行读入
	mov ax,data
	mov ds,ax;通过ax把段地址赋值给ds
	mov ah,01h;准备调用21号中断01指令进行读入
again:
	int 21h
	cmp al,0Dh;比较读入的存在al的值是不是0D，这是判断是否读到了回车
	je end_read;读到了回车就结束读入
	mov s[si],al
	add si,01h
	jmp again;否则就把al的值存入s数组继续读入
end_read:
	mov si,0
	mov di,0;结束读入后，初始化si、di，开始进行转存,由于s数组已初始化为0，所以不必再将最后一位赋值为00h
turn:
	cmp s[si],00h
	je over;如果是0，则转存结束
	cmp s[si],'a'
	jae if_letter;如果大于’a'，进入判断是否是小写字母的函数段
	cmp s[si],' '
	je space;如果是空格，进入跳过转存的函数段
	mov al,s[si]
	mov t[di],al
	add di,01h
	add si,01h
	jmp turn;否则就正常存入t，并进入下一位
space:
	add si,01h
	jmp turn;如果是空格，那么久跳过存入t，进入下一位
if_letter:
	cmp s[si],'z'
	jbe letter;如果大于等于a且小于等于z，判断为小写字母，准备转大写
	mov al,s[si]
	mov t[di],al
	add di,01h
	add si,01h
	jmp turn;否则就正常存入
letter:
	mov al,s[si]
	sub al,20h;转大写
	mov t[di],al
	add di,01h
	add si,01h
	jmp turn;回到循环
over:
	mov di,00h
	mov ah,02h;准备开始调用输出指令
begin:
	cmp t[di],00h
	je end_prt;如果到了0，就输出结束，仅剩0D0A
	mov dl,t[di]
	int 21h
	add di,01h
	jmp begin;回到循环继续输出
end_prt:
	mov dl,0Dh
	int 21h
	mov dl,0Ah
	int 21h;输出0D0A
	mov ah, 4Ch
	int 21h;退出程序
code ends
end main