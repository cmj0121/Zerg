" ZERG Syntax File
" Language:      ZERG
" Maintainer:    cmj <cmj@cmj.tw>
" Last Change:   2016 Jan 21
"

if exists("b:current_syntax")
    finish
endif

let s:cpo_save = &cpo
set cpo&vim

" Comment on ZERG
"
" All comment are starts with # in one line
" If need the multiple-line comment, using long string without assign
" to any identifier, ZERG will skip process this string and consider
" as the comments
"

" Zerg comment and related syntax, like TODO note and documents
syn match   ZERGComment       "#.*$" contains=ZERGToDo,@Spell
syn keyword ZERGToDo          contained XXX TODO FIXME NOTE HACK
syn region  ZERGDoc           start="^\s*\%(>>>\|...\)\s" end="$" contained contains=ALLBUT,ZERGDoc
hi def link ZERGComment        Comment
hi def link ZERGToDo           Todo
hi def link ZERGDoc            Special

" Reserved token - NUMBER
" Number include the integer and floating
syn match   ZERGNumber        "\<\d\+\>"
syn match   ZERGNumber        "\<\d\+[e]-\=\d+\>"
syn match   ZERGNumber        "\<0[oO]\=\o\+\>"
syn match   ZERGNumber        "\<0[xX]\x\+\>"
syn match   ZERGNumber        "\<0[bB][01]\+\>"
syn match   ZERGNumber        "\<\d\+\%([eE][+-]\=\d\+\)\=\>"
hi def link ZERGNumber        Number

" Reserved token - STRING
syn region  ZERGString        start=+\z(['"]\)+ end="\z1" skip="\\\z1" contains=ZERGEscape,ZERGToDo
syn match   ZERGEscape        +\\[abfnrtv'"\\]+ contained
syn match   ZERGEscape        +\\x\x\{2}+ contained
hi def link ZERGString        String
hi def link ZERGEscape        Special

" Reserved words
syn keyword ZERGConstant      none true false
syn keyword ZERGIdentifier    this
syn keyword ZERGStatement     nop as return yield
syn keyword ZERGStatement     func class nextgroup=ZERGFunction skipwhite
syn keyword ZERGLogical       eq not in and or not xor
syn keyword ZERGCondition     if else
syn keyword ZERGRepeat        while break continue
syn match   ZERGFunction      "\%(\(func\|class\)\s\+\)\@<=\w\+" contained
syn keyword ZERGException     try except finally raise
syn keyword ZERGInclude       from import
syn keyword ZERGConstant      TRUE FALSE
syn keyword ZERGBuiltinType   __buffer__
hi def link ZERGConstant      Constant
hi def link ZERGIdentifier    Identifier
hi def link ZERGStatement     Statement
hi def link ZERGLogical       Statement
hi def link ZERGCondition     Statement
hi def link ZERGRepeat        Statement
hi def link ZERGFunction      Function
hi def link ZERGException     Statement
hi def link ZERGInclude       Include
hi def link ZERGConstant      Constant
hi def link ZERGBuiltinType   StorageClass

" Reserved Function
syn keyword ZERGReservedFn    print syscall
hi def link ZERGReservedFn    Function

let &cpo = s:cpo_save
unlet s:cpo_save

" vim: set spell!:
