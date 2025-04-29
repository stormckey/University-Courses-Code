.386
data segment use16
	HEAD db 0EEh
	NECK db 0F7h
	BODY db	00Fh
	WALL db	0B2h
	Red  db	00Ch
	YELLOW	db 00Eh
	GREEN	db 00Ah
	WHITE	db 007h
	UP	dw 4800h
	DOWN	dw 5000h
	LEFT	dw 4B00h
	RIGHT	dw 4D00h
	ESC_	dw 011Bh
	RIGHT_BOUND	db 40d
	BOTTOM_BOUND	db 12d
	DELAY_TICKS	dw 5000d
	maze	db 4000d dup(0)
	trace	db 2000d dup(0)
	mark	db 2000d dup(0)
	bug	db 0EEh,00Ch,0,0,0,0,0F7h,00Eh,0,0,0,0,00Fh,00Ah,0,0,0,0,00Fh,00Ah,0,0,0,0,00Fh,00Ah,0,0,0,0
	N	db 5
	seed	dd 0
data ends

code segment use16
assume cs:code,ds:data
do_delay:;从pcmusic.asm抄过来的delay函数
	dec     cx      
	jz      delay_done
	push    cx      
	mov     ah, 00   
	int     1Ah     
	pop     cx      
	cmp     bx, dx   
	jz      do_delay
delay_done:
	ret

delay:;从pcmusic.asm抄过来的delay函数
	mov     ah,00   
	int     1Ah
	mov     bx, dx   
	mov     cx, 5000d
	call    do_delay    
	ret             

draw_char:
	push bp;搭建框架 bp(+)	oldbp	ip	x	y	shape	color
	mov bp,sp;		0	2	4	6	8	0Ah
	push bx

	mov ax,0B800h
	mov es,ax;es赋值为B800 准备以文本模式显示一个字符
	mov ax,word ptr [bp+6];y
	mov dl,50h;80y
	mul dl
	mov si,ax
	mov ax,word ptr [bp+4];x
	add ax,si;80y+x
	mov dl,2
	mul dx;2（80y+x） 类似的对[80][25]二维数组的处理后面还会出现多次，均是类似方法，只在此注释一次，不再注释
	mov bx,ax;赋值给bx，作为es的偏移地址
	mov al,byte ptr [bp+8];shape
	mov es:[bx],al
	mov al,byte ptr [bp+0Ah];color
	mov es:[bx+1],al

	pop bx
	pop bp;拆解框架 返回
	ret




fill_maze_with_wall:;本代码的for均采用统一格式书写，包括初始化、again标签、结束判断、末尾操作、回到again循环，只在此处注释一次，下同，不再注释
	push bp;搭建框架	x	y	(-)bp(+)	oldbp	ip	
	mov bp,sp;		4	2			0	2
	sub sp,4
	push bx
	mov word ptr[bp-2],0;外部for的初始化
again_y:
	cmp word ptr [bp-2],25d;外部for的结束条件跳转
	je over_again_y
	mov word ptr[bp-4],0;内部for的初始化
again_x:
	cmp word ptr [bp-4],80d;内部for的结束条件跳转
	je over_again_x
	mov al,[bp-2];取二位数组偏移地址
	mov dl,50h
	mul dl
	mov bx,ax
	mov al,[bp-4]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov al,WALL
	mov maze[bx],al;shape赋值为wall
	mov al,WHITE
	mov maze[bx+1],al;color赋值为white
	add word ptr [bp-4],1;内部for的++
	jmp again_x;内部for的循环
over_again_x:
	add word ptr [bp-2],1;外部for的++
	jmp again_y;外部for的循环
over_again_y:
	pop bx
	mov sp,bp
	pop bp
	ret





build_maze:
	push bp;搭建框架	d[4][2]	i	count	flag[4]	(-)bp(+)	oldbp	ip	x0	y0	x1	y1
	mov bp,sp;		0Eh	6	5	4			0	2	4	6	8	0Ah
	sub sp,4
	sub sp,1
	sub sp,1
	sub sp,8
	push bx
	push si
	;变量初始化
	mov dword ptr [bp-4],0
	mov word ptr [bp-6],0
	mov byte ptr [bp-7],-1
	mov byte ptr [bp-8],0
	mov byte ptr [bp-9],0
	mov byte ptr [bp-0Ah],1
	mov byte ptr [bp-0Bh],1
	mov byte ptr [bp-0Ch],0
	mov byte ptr [bp-0Dh],0
	mov byte ptr [bp-0Eh],-1
	;越界则返回
	mov ax,0
	cmp [bp+6],ax
	jbe build_maze_ret
	cmp [bp+4],ax
	jbe build_maze_ret
	mov ax,[bp+6]
	cmp ax,[bp+0Ah]
	ja build_maze_ret
	mov ax,[bp+4]
	cmp ax,[bp+8]
	ja build_maze_ret
	;已经是空格不需要再挖 直接返回
	mov ax,[bp+6];取二位数组偏移地址
	mov dl,50h
	mul dl
	mov bx,ax
	mov ax,[bp+4]
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	cmp maze[bx],' '
	jne not_space_1
	jmp build_maze_ret
not_space_1:
	;循环统计四周空格数
	mov byte ptr [bp-6],0
again_count_space_1:
	cmp byte ptr [bp-6],4
	je over_count_space
	mov bx,[bp+6]
	mov al,byte ptr [bp-6]
	mov ah,0
	mov si,ax
	add si,ax;si赋值为两倍的i，则[bp+si-0Dh]可以取到d[i][1]
	mov al,[bp+si-0Dh] ;[bp+si-0Eh]可以取到d[i][0]，类似方法多次出现，只注释一次
	movsx ax,al
	add bx,ax
	mov ax,bx
	mov dl,50h;取二位数组偏移地址
	mul dl
	mov bx,ax
	mov ax,[bp+4]
	mov dl,[bp+si-0Eh]
	movsx dx,dl
	add ax,dx
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	add byte ptr [bp-6],1
	cmp maze[bx],' ';若是空格 计数count++
	jne again_count_space_1
	add byte ptr [bp-5],1
	jmp again_count_space_1

over_count_space:

	cmp byte ptr [bp-5],2;比较空格数是否大于2 有则不能再挖
	jb space_not_more_than_two
	jmp build_maze_ret

space_not_more_than_two:
	;把当前格赋值为白色空格
	mov ax,word ptr [bp+6]
	mov dl,50h;取二位数组偏移地址
	mul dl
	mov bx,ax
	mov ax,[bp+4]
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov maze[bx],' '
	mov al,WHITE
	mov maze[bx+1],al

	mov byte ptr [bp-5],0
random_dig_road:
	;循环尝试挖四个方向
	cmp byte ptr [bp-5],4
	jae over_dig_road
	;产生随机数，运用注释里的算法
	mov eax,seed
	mov edx,015A4E35h
	mul edx
	add eax,1
	jnc no_edx_add
	add edx,1
no_edx_add:
	shr eax,10h
	mov seed,eax
	and eax,3;对4取余
	mov si,ax
	cmp byte ptr[bp+si-4],1;查看flag 看看这个方向挖过了没有
	jnz not_continue_1
	jmp random_dig_road
not_continue_1:
	mov byte ptr [bp+si-4],1;标记flag表示已经往这个方向挖过了
	add byte ptr[bp-5],1;计数

	push word ptr[bp+0Ah]
	push word ptr[bp+8]
	mov bx,[bp+6];这里把d中的数取出并加上去
	mov ax,si
	add si,ax
	mov al,[bp+si-0Dh] 
	movsx ax,al
	add bx,ax
	push bx
	mov bx,[bp+4]
	mov dl,[bp+si-0Eh]
	movsx dx,dl
	add bx,dx
	push bx
	call build_maze;递归调用挖通道
	add sp,8
	jmp random_dig_road
over_dig_road:
build_maze_ret:
	pop si
	pop bx
	mov sp,bp
	pop bp
	ret
	



search_target_xy:
	push bp;搭建框架	*px	*py	k	d[8][2]	oldx	oldy	(-)bp(+)	oldbp	ip	sp_x	ss_x	sp_y	ss_y
	mov bp,sp;搭建框架	15h	14h	13h	12h	2	1			0	2	4	6	8	0Ah
	sub sp,13h
	sub sp,2h
	push bx
	push si
	;初始化
	mov word ptr [bp-2],0
	mov dword ptr [bp-6],0FFFFFF00h
	mov dword ptr[bp-0Ah],0FF010001h
	mov dword ptr[bp-0Eh],01010100h
	mov dword ptr[bp-12h],11FF00FFh
	mov word ptr [bp-15h],0
	;检查越界，并把*px *py存入变量 方便使用
	mov ax,word ptr [bp+6]
	mov es,ax
	mov ax,word ptr [bp+4]
	mov bx,ax
	mov ax,word ptr es:[bx]
	mov byte ptr [bp-15h],al
	cmp ax,0
	jb search_target_ret_0
	mov dl,RIGHT_BOUND
	mov dh,0
	cmp ax,dx
	ja search_target_ret_0
	mov ax,word ptr [bp+0Ah]
	mov es,ax
	mov ax,word ptr [bp+8]
	mov bx,ax
	mov ax,es:[bx]
	mov byte ptr [bp-14h],al
	cmp ax,0
	jb search_target_ret_0
	mov dl,BOTTOM_BOUND
	mov dh,0
	cmp ax,dx
	ja search_target_ret_0
	;检查是否已经搜索过 用trace标记
	mov al,byte ptr [bp-14h]
	mov dl,50h;取二位数组偏移地址
	mul dl
	mov bx,ax
	mov al,byte ptr [bp-15h]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	cmp trace[bx],1
	je search_target_ret_0
	;标记已经搜过
	mov trace[bx],1
	;如果是空格，则可以把目标点放在此处
	cmp maze[bx],' '
	je search_target_ret_1
	;准备以此为中心向8个方向搜索
	mov al,byte ptr[bp-15h]
	mov byte ptr [bp-2],al
	mov al,byte ptr [bp-14h]
	mov byte ptr [bp-1],al

	mov byte ptr[bp-13h],0
again_find_space:
	;赋值
	cmp byte ptr[bp-13h],8
	jae over_find_space
	mov al,byte ptr [bp-13h]
	mov ah,0
	mov si,ax
	add si,ax
	mov es,word ptr [bp+6]
	mov bx,word ptr [bp+4]
	mov al,byte ptr [bp-12h+si]
	movsx ax,al
	mov es:[bx],ax
	mov al,byte ptr [bp-2]
	movsx ax,al
	add es:[bx],ax
	mov es,word ptr [bp+0Ah]
	mov bx,word ptr [bp+8]
	mov al,byte ptr [bp-11h+si]
	movsx ax,al
	mov es:[bx],ax
	mov al,byte ptr [bp-1]
	movsx ax,al
	add es:[bx],ax
	;调用函数递归搜索
	push word ptr[bp+0Ah]
	push word ptr[bp+8]
	push word ptr[bp+6]
	push word ptr[bp+4]
	call search_target_xy
	add sp,8
	cmp ax,1
	je search_target_ret_1
	add byte ptr [bp-13h],1
	jmp again_find_space
over_find_space:
search_target_ret_0:;返回值为0
	pop si
	pop bx
	mov sp,bp
	pop bp
	mov ax,0
	ret
search_target_ret_1:;返回值为1
	pop si
	pop bx
	mov sp,bp
	pop bp
	mov ax,1
	ret

show_maze:
	push bp;搭建框架	x	y	(-)bp(+）	oldbp	ip	x0	y0	x1	y1
	mov bp,sp;搭建框架	2	1			0	2	4	6	8	0Ah	
	sub sp,2

	mov ax,word ptr [bp+6]
	;外层y控制的for
	mov byte ptr[bp-1],al
again_show_maze_y:
	mov ax,word ptr [bp+0Ah]
	cmp byte ptr [bp-1],al
	ja over_show_maze_y
	mov ax,word ptr [bp+4]
	;内层x控制的for
	mov byte ptr[bp-2],al
again_show_maze_x:
	mov ax,word ptr [bp+8]
	cmp byte ptr [bp-2],al
	ja over_show_maze_x
	;压入参数 调用draw_char函数
	mov al,byte ptr [bp-1]
	mov dl,50h;取二位数组偏移地址
	mul dl
	mov bx,ax
	mov al,byte ptr [bp-2]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov al,maze[bx+1]
	mov ah,0
	push ax
	mov al,maze[bx]
	mov ah,0
	push ax
	mov al,byte ptr [bp-1]
	mov ah,0
	push ax
	mov al,byte ptr [bp-2]
	mov ah,0
	push ax
	call draw_char
	add sp,8

	add byte ptr[bp-2],1
	jmp again_show_maze_x
over_show_maze_x:
	add byte ptr [bp-1],1
	jmp again_show_maze_y
over_show_maze_y:
	
	mov sp,bp
	pop bp
	ret

init_bug:
	push bp;搭建框架	k	nx	ny	d[4][2]	(-)bp(+)	oldbp	ip	x	y	i
	mov bp,sp;搭建框架	0Bh	0Ah	9h	8			0	2	4	6	8	
	sub sp,0Bh
	push bx
	push si
	;初始化
	mov dword ptr [bp-4],0FF000001h
	mov dword ptr [bp-8],010000FFh
	mov word ptr [bp-0Ah],0
	mov byte ptr [bp-0Bh],0
	;检测是否越界
	mov ax,[bp+4]
	cmp ax,0
	jb init_bug_ret_0
	cmp al,RIGHT_BOUND
	ja init_bug_ret_0
	mov ax,[bp+6]
	cmp ax,0
	jb init_bug_ret_0
	cmp al,BOTTOM_BOUND
	ja init_bug_ret_0
	;检测是否是墙
	mov ax,[bp+6]
	mov dx,50h;取二位数组偏移地址
	mul dx
	mov bx,ax
	mov ax,[bp+4]
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov al,maze[bx]
	mov ah,0
	cmp al,WALL
	jnz not_a_wall_0
	jmp init_bug_ret_0
not_a_wall_0:
	;检测是否被之前的节占用
	mov byte ptr [bp-0Bh],0
judge_if_occupied:
	mov al,[bp-0Bh]
	mov ah,0
	cmp ax,[bp+8]
	jae over_judge_if_occupied

	mov al,[bp-0Bh];取bug数组偏移地址
	mov dl,6
	mul dl
	mov si,ax
	mov al,bug[si+2]
	mov ah,0
	cmp ax,[bp+4]
	jne not_occupied
	mov al,bug[si+3]
	mov ah,0
	cmp ax,[bp+6]
	jne not_occupied
	jmp init_bug_ret_0

not_occupied:
	add byte ptr [bp-0Bh],1
	jmp judge_if_occupied
over_judge_if_occupied:
	;未被占用 就把虫子这一节放下
	mov ax,[bp+8]
	mov dx,6;取bug数组偏移地址
	mul dx
	mov si,ax
	mov ax,[bp+4]
	mov bug[si+2],al
	mov ax,[bp+6]
	mov bug[si+3],al
	mov ax,0B800h
	mov es,ax
	mov al,bug[si+3]
	mov ah,0
	mov dx,50h;取二维数组偏移地址
	mul dx
	mov bx,ax
	mov al,bug[si+2]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov ax,[bp+8]
	mov dx,6;取bug数组偏移地址
	mul dx
	mov si,ax
	mov al,es:[bx]
	mov bug[si+4],al
	mov al,es:[bx+1]
	mov bug[si+5],al
	;检查所有节都放完了吗
	cmp word ptr [bp+8],4
	jne not_init_over
	jmp init_bug_ret_1
not_init_over:
	add word ptr [bp+8],1
	;没放完的话 四处搜索 继续放下一节
	mov byte ptr [bp-0Bh],0
again_search_next:
	cmp byte ptr [bp-0Bh],4
	jae over_search_next
	mov al,[bp-0Bh];取d数组和x、y的值进行赋值
	mov ah,0
	mov si,ax
	add si,ax
	mov ax,[bp+4]
	mov [bp-0Ah],ax
	mov ax,[bp-8+si]
	add [bp-0Ah],ax
	mov ax,[bp+6]
	mov [bp-9],ax
	mov ax,[bp-7+si]
	add [bp-9],ax
	push [bp+8]
	mov al,[bp-9]
	mov ah,0
	push ax
	mov al,[bp-0Ah]
	mov ah,0
	push ax
	call init_bug;调用函数寻找下一节放置地点
	add sp,6
	cmp ax,1
	jne not_find_next
	jmp init_bug_ret_1
not_find_next:
	add byte ptr [bp-0Bh],1
	jmp again_search_next
over_search_next:
init_bug_ret_0:
	;0值返回
	mov ax,0
	pop si
	pop bx
	mov sp,bp
	pop bp
	ret
init_bug_ret_1:
	;1值返回
	mov ax,1
	pop si
	pop bx
	mov sp,bp
	pop bp
	ret




draw_bug:
	push bp;搭建框架	i	(-)bp(+）	oldbp	ip	x	y
	mov bp,sp;搭建框架	1			0	2	4	6
	sub sp,1
	push si
	push bx
	;赋值es
	mov ax,0B800h
	mov es,ax
	;循环清楚旧的虫子
	mov byte ptr[bp-1],0
again_clear_oldbug:
	cmp byte ptr [bp-1],5
	jae clear_oldbug_over
	mov al,[bp-1]
	mov ah,0
	mov dl,6;取bug偏移地址
	mul dl
	mov si,ax
	mov al,bug[si+3]
	mov ah,0
	mov dx,50h;计算显卡操作地址
	mul dx
	mov bx,ax
	mov al,bug[si+2]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	;赋值 清除虫子
	mov al,bug[si+4]
	mov ah,0
	mov es:[bx],al
	mov al,bug[si+5]
	mov ah,0
	mov es:[bx+1],al
	add byte ptr [bp-1],1
	jmp again_clear_oldbug
clear_oldbug_over:
	;判断虫子头的位子是否改变 改变的话需要进行更行虫子位置
	mov ax,[bp+4]
	cmp bug[2],al
	jne again_update
	mov ax,[bp+6]
	cmp bug[3],al
	jne again_update
	jmp not_update
	;循环更新虫子各个节的坐标
	mov byte ptr [bp-1],4
again_update:
	cmp byte ptr [bp-1],1
	jb over_update
	mov al,[bp-1]
	mov ah,0
	mov dl,6;取bug偏移地址
	mul dl
	mov si,ax
	;赋值
	mov al,bug[si-4]
	mov bug[si+2],al
	mov al,bug[si-3]
	mov bug[si+3],al
	sub byte ptr [bp-1],1
	jmp again_update
over_update:
	mov ax,[bp+4]
	mov bug[2],al
	mov ax,[bp+6]
	mov bug[3],al
not_update:
	;循环储存被虫子盖住的地方信息
	mov byte ptr [bp-1],0
again_save_hidden:
	cmp byte ptr [bp-1],5
	jae over_save_hidden
	mov al,[bp-1]
	mov ah,0
	mov dl,6;取bug偏移地址
	mul dl
	mov si,ax
	mov al,bug[si+3]
	mov ah,0
	mov dx,50h;计算显卡地址
	mul dx
	mov bx,ax
	mov al,bug[si+2]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov al,es:[bx]
	mov bug[si+4],al
	mov al,es:[bx+1]
	mov bug[si+5],al
	add byte ptr [bp-1],1
	jmp again_save_hidden
over_save_hidden:
	;画新的虫子
	mov byte ptr [bp-1],4
again_new_bug:
	cmp byte ptr [bp-1],0FFh
	je over_new_bug
	mov al,[bp-1]
	mov ah,0
	mov dl,6;bug偏移地址
	mul dl
	mov si,ax
	mov al,bug[si+3]
	mov dl,50h;显卡操作地址
	mul dl
	mov bx,ax
	mov al,bug[si+2]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov al,bug[si]
	mov es:[bx],al
	mov al,bug[si+1]
	mov es:[bx+1],al
	sub byte ptr [bp-1],1
	jmp again_new_bug
over_new_bug:
	pop bx
	pop si
	mov sp,bp
	pop bp
	ret



move_bug:
	push bp;搭建框架	k	nx	ny	d[4][2]	(-)bp(+)	oldbp	ip	x0	y0	x1	y1
	mov bp,sp;搭建框架	0Bh	0Ah	9	8			0	2	4	6	8	0Ah
	sub sp,0Bh
	push bx
	push si
	;变量初始化
	mov dword ptr [bp-4],0FF000001h
	mov dword ptr [bp-8],010000FFh
	mov word ptr [bp-0Ah],0
	mov byte ptr [bp-0Bh],0
	;检查是否越界
	mov ax,[bp+4]
	cmp ax,0
	jbe move_bug_ret_0
	cmp al,RIGHT_BOUND
	ja move_bug_ret_0
	mov ax,[bp+6]
	cmp ax,0
	jbe move_bug_ret_0
	cmp al,BOTTOM_BOUND
	ja move_bug_ret_0
	;检查是不是墙
	mov ax,[bp+6]
	mov dx,50h;计算maze偏移地址
	mul dx
	mov bx,ax
	mov ax,[bp+4]
	add ax,bx
	mov cx,ax
	mov dx,2
	mul dx
	mov bx,ax
	mov al,maze[bx]
	mov ah,0
	cmp al,WALL
	jnz not_a_wall_1
	jmp init_bug_ret_0
not_a_wall_1:
	mov bx,cx
	;比较是否标记过
	cmp byte ptr mark[bx],1
	jnz not_marked
	jmp move_bug_ret_0
not_marked:
	;标记 并纳入trace
	mov byte ptr mark[bx],1
	mov byte ptr trace[bx],1
	push [bp+6]
	push [bp+4]
	;画虫子
	call draw_bug
	add sp,4
	;延时
	call delay
	;检查是否达到目标
	mov ax,[bp+8]
	cmp ax,[bp+4]
	jnz not_arrive
	mov ax,[bp+0Ah]
	cmp ax,[bp+6]
	jnz not_arrive
	jmp move_bug_ret_1
not_arrive:
	;让虫子尝试往某个方向爬
	mov byte ptr [bp-0Bh],0
again_try_move:
	cmp byte ptr [bp-0Bh],4
	jae over_try_move
	;首先对nxny赋值
	mov al,byte ptr [bp-0Bh]
	mov ah,0
	mov si,ax
	add si,ax
	mov ax,[bp+4]
	mov [bp-0Ah],al
	mov ax,[bp-8+si];用bp和si从d中取值
	add [bp-0Ah],al
	mov ax,[bp+6]
	mov [bp-9],al
	mov ax,[bp-7+si]
	add [bp-9],al
	push [bp+0Ah]
	push [bp+8]
	mov al,byte ptr [bp-9]
	mov ah,0
	push ax
	mov al,byte ptr [bp-0Ah]
	mov ah,0
	push ax
	;调用函数试图往某个方向爬
	call move_bug
	add sp,8
	cmp ax,1
	jnz not_arrive_1
	jmp move_bug_ret_1
not_arrive_1:
	push [bp+6]
	push [bp+4]
	;画虫子
	call draw_bug
	add sp,4
	;延迟
	call delay
	add byte ptr [bp-0Bh],1
	jmp again_try_move
over_try_move:
	mov ax,[bp+6]
	mov dx,50h
	mul dx
	mov bx,ax
	mov ax,[bp+4]
	add bx,ax
	mov trace[bx],0
move_bug_ret_0:
	mov ax,0
	pop si
	pop bx
	mov sp,bp
	pop bp
	ret
move_bug_ret_1:
	mov ax,1
	pop si
	pop bx
	mov sp,bp
	pop bp
	ret





clear_bug:
	push bp;搭建框架	i	(-)bp(+)	oldbp	ip
	mov bp,sp;搭建框架	1			0	2
	sub sp,1
	push bx
	push si
	;赋值es
	mov ax,0B800h
	mov es,ax
	;循环清理虫子
	mov byte ptr[bp-1],0
again_clear_bug:
	cmp byte ptr [bp-1],5
	jae over_clear_bug
	mov al,[bp-1]
	mov ah,0
	mov dl,6;取bug的偏移地址
	mul dl
	mov si,ax
	mov al,bug[si+3]
	mov ah,0
	mov dx,50h;取显卡地址
	mul dx
	mov bx,ax
	mov al,bug[si+2]
	mov ah,0
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov al,bug[si+4]
	mov ah,0
	mov es:[bx],al
	mov al,bug[si+5]
	mov ah,0
	mov es:[bx+1],al
	add byte ptr [bp-1],1
	jmp again_clear_bug
over_clear_bug:

	pop si
	pop bx
	mov sp,bp
	pop bp
	ret

show_trace:
	push bp;搭建框架	x	y	(-)bp(+)	oldbp	ip	
	mov bp,sp;搭建框架	2	1			0	2
	sub sp,2
	push bx
	push si
	;外层x循环
	mov byte ptr [bp-2],1
again_draw_x:
	mov al,[bp-2]
	cmp al,RIGHT_BOUND
	ja over_draw_x
	;内层y循环
	mov byte ptr [bp-1],1
again_draw_y:
	mov al,[bp-1]
	cmp al,BOTTOM_BOUND
	ja over_draw_y

	mov al,[bp-1]
	mov dl,50h;计算trace偏移地址
	mul dl
	mov bx,ax
	mov al,[bp-2]
	mov ah,0
	add bx,ax
	cmp trace[bx],1;比较是否是1
	jnz not_draw_road
	mov al,47h
	mov ah,0
	push ax
	mov al,' '
	mov ah,0
	push ax
	mov al,[bp-1]
	mov ah,0
	push ax
	mov al,[bp-2]
	mov ah,0
	push ax
	call draw_char	;画出红线
	add sp,8
not_draw_road:

	add byte ptr [bp-1],1
	jmp again_draw_y
over_draw_y:

	add byte ptr [bp-2],1
	jmp again_draw_x
over_draw_x:
	pop si
	pop bx
	mov sp,bp
	pop bp
	ret





main:
	push bp
	mov bp,sp
	sub sp,4
	mov word ptr [bp-2],12
	mov word ptr [bp-4],40
	mov ax,0
	mov ds,ax
	mov edx,dword ptr ds:[046Ch]
	mov ax,data
	mov ds,ax
	mov seed,edx;取随机种子
	call fill_maze_with_wall;全是墙
	push [bp-2]
	push [bp-4]
	push 1
	push 1
	call build_maze;挖通道
	add sp,8
	sub bp,2
	push ss
	push bp
	sub bp,2
	push ss
	push bp
	add bp,4
	call search_target_xy;确定目标地址
	add sp,8
	mov ax,ds
	mov es,ax
	mov di,offset trace
	mov cx,2000d
	mov al,0
	rep stosb;清除trace
	mov ax,[bp-2]
	mov dx,50h;目标点赋值为青色
	mul dx
	mov bx,ax
	mov ax,[bp-4]
	add ax,bx
	mov dx,2
	mul dx
	mov bx,ax
	mov maze[bx+1],0B7h
	mov al,BOTTOM_BOUND
	mov ah,0
	add ax,1
	push ax
	mov al,RIGHT_BOUND
	mov ah,0
	add ax,1
	push ax
	push 0
	push 0
	call show_maze;显示迷宫
	add sp,8
	mov ah,0
	int 16h
	push 0
	push 1
	push 1
	call init_bug;初始化虫子
	add sp,6
	push [bp-2]
	push [bp-4]
	push 1
	push 1
	call move_bug;让虫子寻找目标点
	add sp,8
	mov ah,0
	int 16h
	call clear_bug;清除虫子
	db 0CCh
	call show_trace;显示路径
	mov ah,0
	int 16h
	pop bp
	mov ah,4Ch
	int 21h;结束
code ends
end main