.386
data segment use16
buffer	db 127
	db 0
	db 100 dup(0);缓冲区 用来读入计算式
ic db 10h;ic ib再比较优先级时会用到 初始化为10h，计算出优先级后赋为012
ib db 10h
outitem dd 256 dup (0);逆波兰输出队列
opitem  db 256 dup(0);符号队列
nout dw 0
nope dw 0
p dw 2
result dd 0
count dw 0
decimal db 100 dup (0)
read db 100 dup (?)
ppp db "+-*/"
pp db "((+-*/"
data ends


code segment use16
assume cs:code , ds:data

reverse_polish_notation:
	mov bx,p;bx始终控制缓冲区下标
	mov di,nout;di始终控制输出队列下标
	mov si,nope;si始终控制符号队列下标
again_reverse:
	cmp buffer[bx],0Dh;检测到回车为结束标志
	je again_out	
	mov bx,p
	mov di,nout
	mov si,nope
	cmp buffer[bx],' '
	jne not_space
	add bx,1
	add p,1;至此是跳过空格
	jmp again_reverse
not_space:
	cmp buffer[bx],'0'
	jb not_a_num
	cmp buffer[bx],'9'
	ja not_a_num
	call fetch_a_num;判断是不是数字 是的话调用函数把数字输出到队列
	jmp again_reverse
not_a_num:
	cmp buffer[bx],'('
	jne not_leftbracket;判断是不是左括号 是的话存入符号队列
	mov opitem[si],'('
	add si,1
	add nope,1
	add bx,1
	add p,1
	jmp again_reverse
not_leftbracket:
	cmp buffer[bx],')'
	jne not_rightbracket;判断是不是右括号 是的话准备进行符号输出
	sub nope,1
	sub si,1
	cmp opitem[si],'('
	je bracket_out_over
again_bracket_out:
	call get_op_order
	movzx eax,ax
	or eax,80000000h;未到左括号 从右到左依次把符号转化输出
	mov outitem[di],eax
	add di,4
	add nout,4
	sub si,1
	sub nope,1
	cmp opitem[si],'(';判断是否达到终点
	je bracket_out_over
	jmp again_bracket_out
bracket_out_over:
	add p,1
	add bx,1
	jmp again_reverse;括号输出结束 继续循环
not_rightbracket:
	cmp nope,0
	jbe sign_end
	call is_lower_privilege;是运算符号先比较优先级
	cmp al,0
	je sign_end;优先级大 直接跳转输出 否则先循环把比他小的输出完
	sub nope,1
	sub si,1
	call get_op_order
	movzx eax,ax
	or eax,80000000h
	mov outitem[di],eax
	add di,4
	add nout,4
	cmp nope,0
	jbe sign_end
	call is_lower_privilege
	cmp al,0
	je sign_end
	jmp not_rightbracket
sign_end:
	mov al,buffer[bx]
	mov opitem[si],al
	add si,1
	add nope,1
	add bx,1
	add p,1
	jmp again_reverse
again_out:
	cmp nope,0;最后若符号队列中还有剩余 继续输出
	je reverse_ret
	sub nope,1
	sub si,1
	call get_op_order;
	movzx eax,ax
	or eax,80000000h
	mov outitem[di],eax
	add di,4
	add nout,4
	jmp again_out
reverse_ret:
	ret



get_op_order:;把加减乘除转化0123，返回ax
	mov bp,0
	mov si,nope
again_order:
	mov al,ppp[bp]
	cmp opitem[si],al
	je order_ret
	add bp,1
	jmp again_order
order_ret:
	mov ax,bp
	ret



fetch_a_num:;获取数字 这里要处理的是连续数字要十进制转化十六进制存入
	mov bx,p
	mov eax,0
	mov bp,0
	mov di,nout
again_fetch:
	cmp buffer[bx],'0'
	jb not_num
	cmp buffer[bx],'9'
	ja not_num
	mov cl,buffer[bx]
	sub cl,'0'
	mov read[bp],cl;首先逐字节存入read队列中
	add bp,1
	add bx,1
	jmp again_fetch
not_num:
	cmp bp,1
	je simple_mode;如果只有一位 直接存入
	mov dx,0
	push bx
	mov bx,0Ah
	mov dx,0
	mov count,bp
	sub count,1
again_generate:;多于一位 每个位数循环乘以10再相加存入
	sub bp,1
	mov al,read[bp]
	movzx ax,al
	push count
again_multiple:
	sub count,1
	cmp count,0
	jbe end_multiple
	mul bx
	jmp again_multiple
end_multiple:
	pop count
	add count,1
	add word ptr outitem[di],ax
	add word ptr outitem[di+2],dx
	cmp bp,0
	jne again_generate
	pop bx
	mov p,bx
	add di,4
	mov nout,di
	jmp fetch_ret
simple_mode:
	sub bp,1
	mov al,read[bp]
	movzx eax,al
	mov outitem[di],eax
	add di,4
	add nout,4
	mov p,bx
fetch_ret:
	ret



is_lower_privilege:;比较优先级 这里只用一位数组 01为左括号 23为加减 45为乘除
	mov ic,10h;除以二得到012优先级 进行比较 返回al 01表示是否是低优先级
	mov ib,10h
	mov si,nope
	mov bx,p
	mov bp,0
	mov cx,0
again_cmp_privilege:
	cmp ic,10h
	jne not_give_c
	mov al,buffer[bx]
	cmp pp[bp],al
	jne not_give_c
	mov ax,bp
	mov dl,2
	div dl
	mov ic,al
	add cx,1;为ic赋优先级
not_give_c:
	cmp ib,10h
	jne not_give_b
	mov al,opitem[si-1]
	cmp pp[bp],al
	jne not_give_b
	mov ax,bp
	mov dl,2
	div dl
	mov ib,al
	add cx,1;为ib赋优先级
not_give_b:
	add bp,1
	cmp cx,2
	jne again_cmp_privilege
	mov al,ic
	mov ah,ib
	cmp al,ah
	mov al,0
	ja privilege_ret
	mov al,1
privilege_ret:
	ret

compute:
	cmp nout,0
	je compute_over

	mov di,0
not_find_sign:
	mov eax,outitem[di]
	and eax,80000000h
	cmp eax,0
	jne find_sign;计算 首先寻找第一个符号
	add di,4
	jmp not_find_sign
find_sign:
	mov eax,outitem[di]
	and eax,7FFFFFFFh
	mov ecx,outitem[di-8]
	mov edx,outitem[di-4]
	cmp eax,0;找到了 开始按照0123寻找对应运算
	je _plus
	cmp eax,1
	je _minus
	cmp eax,2
	je _mul
	cmp eax,3
	je _div
_plus:
	call plus_
	jmp _save
_minus:
	call minus_
	jmp _save
_mul:
	call mul_
	jmp _save
_div:
	call div_
	jmp _save
_save:;算完后 结果eax存入第一个数的位置
	mov outitem[di-8],eax
	push di
	mov ax,nout
	mov dl,4
	div dl
	push ax
	mov ax,di
	div dl
	pop cx
	sub cx,ax
again_mov:;循环前移 队列减少2个
	cmp cx,0
	je mov_over
	mov eax,outitem[di+4]
	mov outitem[di-4],eax
	add di,4
	sub cx,1
	jmp again_mov
mov_over:
	sub nout,8
	pop di
	sub di,4
compute_over:;计算结束 结果就在输出队列第一位
	cmp nout,4
	jne not_find_sign
	ret

plus_:
	add ecx,edx
	mov eax,ecx
	ret

minus_:
	sub ecx,edx
	mov eax,ecx
	ret

mul_:
	mov eax,ecx
	mul edx
	ret

div_:
	mov eax,ecx
	mov ecx,edx
	mov edx,0
	div ecx
	ret

decimal_out:;十进制输出方法是除以10取余数 存入数组中 最后反向输出
	mov ax,word ptr outitem[0]
	mov dx,word ptr outitem[2]
	mov cl,0Ah
	mov si,0
again_div:
	div cx
	add dx,'0'
	mov decimal[si],dl
	add si,1
	mov dx,0
	cmp ax,0
	jne again_div
decimal_print:
	cmp si,0
	je decimal_ret
	sub si,1
	mov dl,decimal[si]
	mov ah,02h
	int 21h
	jmp decimal_print
decimal_ret:
	ret

hex_out:;16进制输出 方法是每次用al取出两位 除以10h分离两个数 再分别按照数字字母输出
	mov si,3
again_hex:
	mov al,byte ptr outitem[si]
	mov ah,00h
	mov cl,10h
	div cl
	cmp al,9
	ja char_print_al
	mov dl,al
	add dl,'0'
	push ax
	mov ah,02h
	int 21h
	pop ax
	jmp al_end
char_print_al:
	mov dl,al
	add dl,'A'-0Ah
	push ax
	mov ah,02h
	int 21h
	pop ax
al_end:

	cmp ah,9
	ja char_print_ah
	mov dl,ah
	add dl,'0'
	mov ah,02h
	int 21h
	jmp ah_end
char_print_ah:
	mov dl,ah
	add dl,'A'-0Ah
	mov ah,02h
	int 21h
ah_end:
	cmp si,0
	je hex_ret
	sub si,1
	jmp again_hex
hex_ret:
	mov dl,'h'
	mov ah,02
	int 21h
	ret

binary_out:;二进制输出 循环左移 每次处理16位 每4位输出一个空格
	mov si,2
	mov bx,word ptr outitem[si]
	mov cx,16
	mov count,0
again_binary_1:
	cmp cx,0
	je binary_ret_1
	sub cx,1
	shl bx,1
	jnc print_zero_1
	mov dl,'1'
	mov ah,2
	int 21h
	add count,1
	cmp count,4
	jne again_binary_1
	call space
	mov count,0
	jmp again_binary_1
print_zero_1:
	mov dl,'0'
	mov ah,2
	int 21h
	add count,1
	cmp count,4
	jne again_binary_1
	call space
	mov count,0
	jmp again_binary_1
binary_ret_1:
	mov si,0
	mov bx,word ptr outitem[si]
	mov cx,16
again_binary_2:
	cmp cx,0
	je binary_ret_2
	sub cx,1
	shl bx,1
	jnc print_zero_2
	mov dl,'1'
	mov ah,2
	int 21h
	add count,1
	cmp count,4
	jne again_binary_2
	call space
	mov count,0
	jmp again_binary_2
print_zero_2:
	mov dl,'0'
	mov ah,2
	int 21h
	add count,1
	cmp count,4
	jne again_binary_2
	call space
	mov count,0
	jmp again_binary_2
	jmp again_binary_2
binary_ret_2:
	mov dl,'B'
	mov ah,2
	int 21h
	ret

enter_:;回车换行
	mov ah,02h
	mov dl,0Dh
	int 21h
	mov dl,0Ah
	int 21h
	ret

space:;输出空格
	cmp cx,0
	jne goon
	cmp si,0
	jne goon
	jmp justover
goon:;这个是检测如果是最后一位不用空格
	mov dl,' '
	mov ah,2
	int 21h
justover:
	ret

main:
	mov ax,data
	mov ds,ax
	mov dx,offset buffer
	mov ah,0Ah
	int 21h
	call reverse_polish_notation;逆波兰
	call compute;计算
	call enter_	
	call decimal_out;十进制输出
	call enter_
	call hex_out;十六进制输出
	call enter_
	call binary_out;二进制输出
	mov ah,4Ch
	int 21h
code ends
end main