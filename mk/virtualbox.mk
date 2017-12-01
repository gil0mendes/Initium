virtualbox: $(iso)
	@echo "Delete VM"
	-$(VBM) unregistervm Initium --delete; \
	if [ $$? -ne 0 ]; \
	then \
		if [ -d "$$HOME/VirtualBox VMs/Initium" ]; \
		then \
			@echo "Initium directory exists, deleting..."; \
			rm -rf "$$HOME/VirtualBox VMs/Initium"; \
		fi \
	fi
	@echo "Create VM"
	$(VBM) createvm --name Initium --register
	@echo "Set Configuration"
	$(VBM) modifyvm Initium --memory 2048
	$(VBM) modifyvm Initium --vram 32
	$(VBM) modifyvm Initium --firmware efi
	#$(VBM) modifyvm Initium --uart1 0x3F8 4
	#$(VBM) modifyvm Initium --uartmode1 file "$(build_dir)/serial.log"
	$(VBM) modifyvm Initium --usb off
	$(VBM) modifyvm Initium --keyboard ps2
	@echo "Attach CD-ROm"
	$(VBM) storagectl Initium --name IDE --add ide
	$(VBM) storageattach Initium --storagectl IDE --port 0 --device 0 --type dvddrive --medium $(iso)
	@echo "Run VM"
	$(VBM) startvm Initium
