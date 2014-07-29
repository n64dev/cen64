.code
fpu_cmp_olt_64 proc
  movsd xmm0, QWORD PTR [rcx]
  movsd xmm1, QWORD PTR [rdx]
  comisd xmm1, xmm0
  seta dl
  setnp al
  and al, dl
  ret
fpu_cmp_olt_64 endp
end

