.code
fpu_cmp_un_64 proc
  movsd xmm0, QWORD PTR [rcx]
  movsd xmm1, QWORD PTR [rdx]
  comisd xmm1, xmm0
  setp al
  ret
fpu_cmp_un_64 endp
end

