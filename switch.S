.global switch_context          // `void switch_context(Context** old, Context* new)`
switch_context:
        mov 4(%esp), %eax       // `Context** const old`
        mov 8(%esp), %edx       // `Context* const new`

        // Save old registers. Note that `esp` is saved as the address of the
        // `Context` itself.
        push %ebp
        push %ebx
        push %esi
        push %edi

        // Switch stacks.
        mov %esp, (%eax)
        mov %edx, %esp

        // Load new registers.
        pop %edi
        pop %esi
        pop %ebx
        pop %ebp
        ret
