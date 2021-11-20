CFLAGS = -g -m32 -nostdlib -fno-builtin -fno-stack-protector -Isrc \
             -nostartfiles -nodefaultlibs -Wall -Wextra -Werror
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
	build/ihandle.o

build/os.iso: build/ build/kernel
	grub-mkrescue -o $@ iso

build/kernel: ${OBJECTS} link.ld
	ld -T link.ld -m elf_i386 ${OBJECTS} -o $@
	cp $@ ./iso/boot/kernel

build/%.o: src/%.s
	nasm -g -f elf32 -o $@ $<

build/%.o: src/%.c
	gcc ${CFLAGS} -c $< -o $@

.PHONY qemu: build/os.iso
	qemu-system-i386 -cdrom $<

build/:
	mkdir -p build

clean:
	rm -rf build

build/kheaptest: src/kheap.c test/kheaptest.c src/ordlist.c
	gcc -Wall -Werror -g -Isrc -m32 test/kheaptest.c src/ordlist.c src/kheap.c -o $@

test: build/kheaptest
	./$<

debug: build/os.iso
	qemu-system-i386 -gdb tcp::1234 -cdrom $<