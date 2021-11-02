CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
             -nostartfiles -nodefaultlibs -Wall -Wextra -Werror

OBJECTS = build/main.o build/loader.o

build/os.iso: build/kernel
	grub-mkrescue -o $@ iso

build/kernel: ${OBJECTS}
	ld -T link.ld -m elf_i386 ${OBJECTS} -o $@
	cp $@ ./iso/boot/kernel

build/%.o: %.s
	nasm -f elf32 -o $@ $<

build/%.o: %.c
	gcc ${CFLAGS} -c $< -o $@

.PHONY qemu: build/os.iso
	qemu-system-i386 -cdrom $<

clean:
	rm -rf build