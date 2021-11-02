build/os.iso: build/kernel
	grub-mkrescue -o $@ iso

build/kernel: build/loader.o link.ld
	ld -T link.ld -m elf_i386 $< -o $@
	cp $@ ./iso/boot/kernel

build/loader.o: loader.s
	nasm -f elf32 -o $@ $<

.PHONY qemu: build/os.iso
	qemu-system-i386 -cdrom $< -D build/log.txt