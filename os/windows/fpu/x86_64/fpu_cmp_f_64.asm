.code
fpu_cmp_f_64 proc
  mov QWORD PTR [rsp-0x8], rcx
  movsd xmm0, QWORD PTR [rsp-0x8]
  mov QWORD PTR [rsp-0x8], rdx
  movsd xmm1, QWORD PTR [rsp-0x8]
  comisd xmm1, xmm0
  ret
fpu_cmp_f_64 endp
end

