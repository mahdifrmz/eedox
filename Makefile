CFLAGS = -g -Isrc -ffreestanding -Wall -Wextra -Werror
CUSERFLAGS = -Iuser -ffreestanding -Wall -Wextra -Werror
OBJECTS =\
	build/multsk.o \
	build/kqueue.o \
	build/util.o \
	build/kutil.o \
	build/vec.o \
	build/kstring.o \
	build/bitset.o \
	build/paging.o \
	build/ordlist.o \
	build/kheap.o \
	build/main.o \
	build/asm.o \
	build/loader.o \
	build/terminal.o \
	build/gdt.o \
	build/idt.o \
	build/ihandle.o \
	build/syscall.o \
	build/lock.o \
	build/kb.o \
	build/ata.o \
	build/pathbuf.o \
	build/fs.o \
	build/prog.o \
	build/descriptor.o \
	build/heapwatch.o \
	build/trace.o

USER_BINS =\
	build/user/sh \
	build/user/init \
	build/user/echo \
	build/user/cat \
	build/user/ls \
	build/user/tee \
	build/user/mkdir
STDLIB_SRC=\
	user/stdlib.c \
	user/stdlib.h \
	src/util.c \
	user/stdlib-internal.h \
	user/asmlib.s

QEMU_FLAGS = -drive file=build/vdsk.img,format=raw,index=0,media=disk

build/os.iso: build/kernel build/vdsk.img
	grub-mkrescue -o $@ iso

build/vdsk.img: ${USER_BINS} fsgen.py
	qemu-img create -fraw build/vdsk.img 16m
	python3 fsgen.py build/binaries ${USER_BINS}
	dd if=build/binaries of=build/vdsk.img conv=notrunc

build/kernel: ${OBJECTS} link.ld trace.py
	ld -T link.ld -m elf_i386 ${OBJECTS} -o $@
	python3 trace.py | dd if=/dev/stdin of=build/kernel bs=4096 count=8 seek=9 conv=notrunc 
	cp $@ ./iso/boot/kernel

build/user/libstd.a: ${STDLIB_SRC}
	i686-elf-gcc ${CUSERFLAGS} -c user/stdlib.c -o build/user/stdlib.o 
	i686-elf-gcc ${CUSERFLAGS} -Isrc -c src/util.c -o build/user/util.o
	nasm -f elf32 -o build/user/asmlib.o user/asmlib.s
	ar rcs build/user/libstd.a build/user/asmlib.o build/user/stdlib.o build/user/util.o

build/user/%: user/%.c build/user/libstd.a
	i686-elf-gcc ${CUSERFLAGS} -c $< -o $@.o
	ld --section-start=.text=0x01400000 -m elf_i386 --entry=startup -o $@ $@.o -Lbuild/user -lstd

build/%.o: src/%.s
	nasm -g -f elf32 -o $@ $<

build/%.o: src/%.c
	i686-elf-gcc ${CFLAGS} -c $< -o $@

.PHONY qemu: build/os.iso
	qemu-system-i386 ${QEMU_FLAGS} -cdrom $<

build/:
	mkdir -p build

build/user:
	mkdir -p build/user

clean:
	rm -rf build
	mkdir build
	mkdir build/user
img-reset:
	rm -f build/vdsk.img

debug: build/os.iso
	qemu-system-i386 ${QEMU_FLAGS} -gdb tcp::1234 -cdrom $<