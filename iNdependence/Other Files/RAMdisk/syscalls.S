.global _mlock
.global _stat
.global _swtch_pri
.global _mount
.global _chroot
.global _chmod
.global _execve
.global _reboot

_mlock:
  mov r12, #0xCB
  swi 128
  bx lr

_stat:
  mov r12, #0xBC
  swi 128
  bx lr

_swtch_pri:
  mvn r12, #0x3a
  swi 128
  bx lr

_mount:
  mov r12, #0xA7
  swi 128
  bx lr

_chroot:
  mov r12, #0x3D
  swi 128
  bx lr

_chmod:
  mov r12, #0xF
  swi 128
  bx lr

_execve:
  mov r12, #0x3B
  swi 128
  bx lr

_reboot:
  mov r12, #0x37
  swi 128
  bx lr
  
