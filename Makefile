CFLAGS = -g -Isrc -ffreestanding -Wall -Wextra -Werror
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
	i686-elf-gcc ${CFLAGS} -c $< -o $@

.PHONY qemu: build/os.iso
	qemu-system-i386 -cdrom $<

build/:
	mkdir -p build

clean:
	rm -rf build
	
debug: build/os.iso
	qemu-system-i386 -gdb tcp::1234 -cdrom $<