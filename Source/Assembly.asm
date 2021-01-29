ifdef PLATFORM_X86
    .model flat, c
endif

.code


; externals
;

extern Proxy_Initialize: proc


; pushall & popall macros
;

pushall macro
    ifdef PLATFORM_X86
        pushad
    elseifdef PLATFORM_X64
        push	rax
        push	rcx
        push	rdx
        push	rbx
        push	rsp
        push	rbp
        push	rsi
        push	rdi
        push	r8
        push	r9
        push	r10
        push	r11
        push	r12
        push	r13
        push	r14
        push	r15
    else
        .err <Project configuration error.>
    endif
endm

popall macro
    ifdef PLATFORM_X86
        popad
    elseifdef PLATFORM_X64
        pop		r15
        pop		r14
        pop		r13
        pop		r12
        pop		r11
        pop		r10
        pop		r9
        pop		r8
        pop		rdi
        pop		rsi
        pop		rbp
        pop		rsp
        pop		rbx
        pop		rdx
        pop		rcx
        pop		rax
    else
        .err <Project configuration error.>
    endif
endm


; callback macro
;

original_exports_callbacker macro callback: req
    ifdef OS_WIN10
        callback    GetFileVersionInfoA,        1
        callback    GetFileVersionInfoByHandle, 2
        callback    GetFileVersionInfoExA,      3
        callback    GetFileVersionInfoExW,      4
        callback    GetFileVersionInfoSizeA,    5
        callback    GetFileVersionInfoSizeExA,  6
        callback    GetFileVersionInfoSizeExW,  7
        callback    GetFileVersionInfoSizeW,    8
        callback    GetFileVersionInfoW,        9
        callback    VerFindFileA,               10
        callback    VerFindFileW,               11
        callback    VerInstallFileA,            12
        callback    VerInstallFileW,            13
        callback    VerLanguageNameA,           14
        callback    VerLanguageNameW,           15
        callback    VerQueryValueA,             16
        callback    VerQueryValueW,             17
    elseifdef OS_WIN7
        callback    GetFileVersionInfoA,        1
        callback    GetFileVersionInfoByHandle, 2
        callback    GetFileVersionInfoExW,      3
        callback    GetFileVersionInfoSizeA,    4
        callback    GetFileVersionInfoSizeExW,  5
        callback    GetFileVersionInfoSizeW,    6
        callback    GetFileVersionInfoW,        7
        callback    VerFindFileA,               8
        callback    VerFindFileW,               9
        callback    VerInstallFileA,            10
        callback    VerInstallFileW,            11
        callback    VerLanguageNameA,           12
        callback    VerLanguageNameW,           13
        callback    VerQueryValueA,             14
        callback    VerQueryValueW,             15
    else
        .err <Project configuration error.>
    endif
endm


; extern original exported function address variables
;

extern_original_function_variable macro name: req, ordinal: req
    extern @catstr(<Proxy_OEFn>, name) : ptr
endm

original_exports_callbacker extern_original_function_variable


; implement proxy functions
;

impl_proxy_function macro name: req, ordinal: req
    @catstr(<asm_proxy_>, name) proc
        pushall
        call Proxy_Initialize
        popall
        jmp @catstr(<Proxy_OEFn>, name)
    @catstr(<asm_proxy_>, name) endp
endm

original_exports_callbacker impl_proxy_function


end
