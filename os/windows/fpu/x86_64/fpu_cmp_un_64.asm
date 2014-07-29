.code
fpu_cmp_un_64 proc
  mov QWORD PTR [rsp-8], rcx
  movsd xmm0, QWORD PTR [rsp-8]
  mov QWORD PTR [rsp-8], rdx
  movsd xmm1, QWORD PTR [rsp-8]
  comisd xmm1, xmm0
  setp al
  ret
fpu_cmp_un_64 endp
end

