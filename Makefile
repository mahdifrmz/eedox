CFLAGS = -m32 -nostdlib -fno-builtin -fno-stack-protector -Isrc \
             -nostartfiles -nodefaultlibs -Wall -Wextra -Werror
# -nostdinc
OBJECTS = build/ordlist.o build/kheap.o build/main.o build/loader.o build/asm.o build/terminal.o build/gdt.o build/idt.o build/ihandle.o

build/os.iso: build/ build/kernel
	grub-mkrescue -o $@ iso

build/kernel: ${OBJECTS} link.ld
	ld -T link.ld -m elf_i386 ${OBJECTS} -o $@
	cp $@ ./iso/boot/kernel

build/%.o: src/%.s
	nasm -f elf32 -o $@ $<

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