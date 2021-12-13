CFLAGS = -g -Isrc -ffreestanding -Wall -Wextra -Werror
CUSERFLAGS = -g -Iuser -ffreestanding -Wall -Wextra -Werror
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
	build/ata.o
	
	# build/fs.o
	# build/ide.o
	# build/lock.o 
	
USER_BINS =\
	build/user/helloworld

STDLIB_SRC=\
	user/stdlib.c \
	user/stdlib.h \
	src/util.c \
	user/stdlib-internal.h \
	user/asmlib.s

QEMU_FLAGS = -drive file=build/vdsk.img,format=raw,index=0,media=disk

build/os.iso: build/kernel build/vdsk.img
	grub-mkrescue -o $@ iso

build/vdsk.img: ${USER_BINS}
	qemu-img create -fraw build/vdsk.img 16m

build/kernel: ${OBJECTS} link.ld
	ld -T link.ld -m elf_i386 ${OBJECTS} -o $@
	cp $@ ./iso/boot/kernel

build/user/libstd.a: ${STDLIB_SRC}
	i686-elf-gcc ${CUSERFLAGS} -c user/stdlib.c -o build/user/stdlib.o 
	i686-elf-gcc ${CFLAGS} -c src/util.c -o build/user/util.o
	nasm -f elf32 -o build/user/asmlib.o user/asmlib.s
	ar rcs build/user/libstd.a build/user/asmlib.o build/user/stdlib.o build/user/util.o

build/user/%: user/%.c build/user/libstd.a
	i686-elf-gcc ${CUSERFLAGS} -c $< -o $@.o
	ld -m elf_i386 --entry=main -o $@ $@.o -Lbuild/user -lstd

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
	
debug: build/os.iso
	qemu-system-i386 ${QEMU_FLAGS} -gdb tcp::1234 -cdrom $<