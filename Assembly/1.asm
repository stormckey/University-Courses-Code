data segment
s db 100 dup(0)
t db 100 dup(0)
data ends
;�����Ǳ�׼�����鶨�� Ӧ����dup�ظ�100��
code segment
assume cs:code , ds:data;����Ǵӿα��ϳ�����
main:
	mov si,00h;si��s������±꣬�ȳ�ʼ��Ϊ0��׼�����ж���
	mov ax,data
	mov ds,ax;ͨ��ax�Ѷε�ַ��ֵ��ds
	mov ah,01h;׼������21���ж�01ָ����ж���
again:
	int 21h
	cmp al,0Dh;�Ƚ϶���Ĵ���al��ֵ�ǲ���0D�������ж��Ƿ�����˻س�
	je end_read;�����˻س��ͽ�������
	mov s[si],al
	add si,01h
	jmp again;����Ͱ�al��ֵ����s�����������
end_read:
	mov si,0
	mov di,0;��������󣬳�ʼ��si��di����ʼ����ת��,����s�����ѳ�ʼ��Ϊ0�����Բ����ٽ����һλ��ֵΪ00h
turn:
	cmp s[si],00h
	je over;�����0����ת�����
	cmp s[si],'a'
	jae if_letter;������ڡ�a'�������ж��Ƿ���Сд��ĸ�ĺ�����
	cmp s[si],' '
	je space;����ǿո񣬽�������ת��ĺ�����
	mov al,s[si]
	mov t[di],al
	add di,01h
	add si,01h
	jmp turn;�������������t����������һλ
space:
	add si,01h
	jmp turn;����ǿո���ô����������t��������һλ
if_letter:
	cmp s[si],'z'
	jbe letter;������ڵ���a��С�ڵ���z���ж�ΪСд��ĸ��׼��ת��д
	mov al,s[si]
	mov t[di],al
	add di,01h
	add si,01h
	jmp turn;�������������
letter:
	mov al,s[si]
	sub al,20h;ת��д
	mov t[di],al
	add di,01h
	add si,01h
	jmp turn;�ص�ѭ��
over:
	mov di,00h
	mov ah,02h;׼����ʼ�������ָ��
begin:
	cmp t[di],00h
	je end_prt;�������0���������������ʣ0D0A
	mov dl,t[di]
	int 21h
	add di,01h
	jmp begin;�ص�ѭ���������
end_prt:
	mov dl,0Dh
	int 21h
	mov dl,0Ah
	int 21h;���0D0A
	mov ah, 4Ch
	int 21h;�˳�����
code ends
end main