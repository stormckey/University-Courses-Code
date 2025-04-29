.386
data segment use16
buffer	db 127
	db 0
	db 100 dup(0);������ �����������ʽ
ic db 10h;ic ib�ٱȽ����ȼ�ʱ���õ� ��ʼ��Ϊ10h����������ȼ���Ϊ012
ib db 10h
outitem dd 256 dup (0);�沨���������
opitem  db 256 dup(0);���Ŷ���
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
	mov bx,p;bxʼ�տ��ƻ������±�
	mov di,nout;diʼ�տ�����������±�
	mov si,nope;siʼ�տ��Ʒ��Ŷ����±�
again_reverse:
	cmp buffer[bx],0Dh;��⵽�س�Ϊ������־
	je again_out	
	mov bx,p
	mov di,nout
	mov si,nope
	cmp buffer[bx],' '
	jne not_space
	add bx,1
	add p,1;�����������ո�
	jmp again_reverse
not_space:
	cmp buffer[bx],'0'
	jb not_a_num
	cmp buffer[bx],'9'
	ja not_a_num
	call fetch_a_num;�ж��ǲ������� �ǵĻ����ú������������������
	jmp again_reverse
not_a_num:
	cmp buffer[bx],'('
	jne not_leftbracket;�ж��ǲ��������� �ǵĻ�������Ŷ���
	mov opitem[si],'('
	add si,1
	add nope,1
	add bx,1
	add p,1
	jmp again_reverse
not_leftbracket:
	cmp buffer[bx],')'
	jne not_rightbracket;�ж��ǲ��������� �ǵĻ�׼�����з������
	sub nope,1
	sub si,1
	cmp opitem[si],'('
	je bracket_out_over
again_bracket_out:
	call get_op_order
	movzx eax,ax
	or eax,80000000h;δ�������� ���ҵ������ΰѷ���ת�����
	mov outitem[di],eax
	add di,4
	add nout,4
	sub si,1
	sub nope,1
	cmp opitem[si],'(';�ж��Ƿ�ﵽ�յ�
	je bracket_out_over
	jmp again_bracket_out
bracket_out_over:
	add p,1
	add bx,1
	jmp again_reverse;����������� ����ѭ��
not_rightbracket:
	cmp nope,0
	jbe sign_end
	call is_lower_privilege;����������ȱȽ����ȼ�
	cmp al,0
	je sign_end;���ȼ��� ֱ����ת��� ������ѭ���ѱ���С�������
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
	cmp nope,0;��������Ŷ����л���ʣ�� �������
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



get_op_order:;�ѼӼ��˳�ת��0123������ax
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



fetch_a_num:;��ȡ���� ����Ҫ���������������Ҫʮ����ת��ʮ�����ƴ���
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
	mov read[bp],cl;�������ֽڴ���read������
	add bp,1
	add bx,1
	jmp again_fetch
not_num:
	cmp bp,1
	je simple_mode;���ֻ��һλ ֱ�Ӵ���
	mov dx,0
	push bx
	mov bx,0Ah
	mov dx,0
	mov count,bp
	sub count,1
again_generate:;����һλ ÿ��λ��ѭ������10����Ӵ���
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



is_lower_privilege:;�Ƚ����ȼ� ����ֻ��һλ���� 01Ϊ������ 23Ϊ�Ӽ� 45Ϊ�˳�
	mov ic,10h;���Զ��õ�012���ȼ� ���бȽ� ����al 01��ʾ�Ƿ��ǵ����ȼ�
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
	add cx,1;Ϊic�����ȼ�
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
	add cx,1;Ϊib�����ȼ�
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
	jne find_sign;���� ����Ѱ�ҵ�һ������
	add di,4
	jmp not_find_sign
find_sign:
	mov eax,outitem[di]
	and eax,7FFFFFFFh
	mov ecx,outitem[di-8]
	mov edx,outitem[di-4]
	cmp eax,0;�ҵ��� ��ʼ����0123Ѱ�Ҷ�Ӧ����
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
_save:;����� ���eax�����һ������λ��
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
again_mov:;ѭ��ǰ�� ���м���2��
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
compute_over:;������� �������������е�һλ
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

decimal_out:;ʮ������������ǳ���10ȡ���� ���������� ��������
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

hex_out:;16������� ������ÿ����alȡ����λ ����10h���������� �ٷֱ���������ĸ���
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

binary_out:;��������� ѭ������ ÿ�δ���16λ ÿ4λ���һ���ո�
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

enter_:;�س�����
	mov ah,02h
	mov dl,0Dh
	int 21h
	mov dl,0Ah
	int 21h
	ret

space:;����ո�
	cmp cx,0
	jne goon
	cmp si,0
	jne goon
	jmp justover
goon:;����Ǽ����������һλ���ÿո�
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
	call reverse_polish_notation;�沨��
	call compute;����
	call enter_	
	call decimal_out;ʮ�������
	call enter_
	call hex_out;ʮ���������
	call enter_
	call binary_out;���������
	mov ah,4Ch
	int 21h
code ends
end main