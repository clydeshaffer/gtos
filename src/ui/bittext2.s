.segment "ZEROPAGE"
bittext_pat:
.byte 0, 0

.segment "BSS"
_bittext_x:
    .byte 0

.segment "DATA"
    ;A should be char
    SEC
    SBC #32 ;space character
    ASL
    STA bittext_pat
    LDA #0
    ROL
    STA bittext_pat+1
    LDA bittext_pat
    ASL
    STA bittext_pat
    LDA bittext_pat+1
    ROL
    STA bittext_pat+1
    CLC
    LDA loadFontPat+1
    ADC bittext_pat
    STA loadFontPat+1
    LDA loadFontPat+2
    ADC #0
    STA loadFontPat+2
    LDX #0

;LoaD from system_font_monochrome ;;this gets added to by previous instruction and restored at end
loadFontPat:
    LDA _system_font_monochrome, x

    LDA _bittext_color
    LDY _bittext_x
    BBR0 bittext_pat, *+6
text_row_0:
    STA 0x4000, y
    BBR1 bittext_pat, *+6
text_row_1:
    STA 0x4080, y
    BBR2 bittext_pat, *+6
text_row_2:
    STA 0x4100, y
    BBR3 bittext_pat, *+6
text_row_3:
    STA 0x4180, y
    BBR4 bittext_pat, *+6
text_row_4:
    STA 0x4200, y
    BBR5 bittext_pat, *+6
text_row_5:
    STA 0x4280, y
    BBR6 bittext_pat, *+6
text_row_6:
    STA 0x4300, y
    BBR7 bittext_pat, *+6
text_row_7:
    STA 0x4380, y


    RTS
    
.segment "CODE"
_bittext_set_x:
    TAY
    LDA _bittext_x
    AND #$80
    STA _bittext_x
    TYA
    ORA _bittext_x
    STA _bittext_x
    RTS

_bittext_set_y:
    TAY
    LSR
    CLC
    ADC #40
    STA text_row_0+1
    STA text_row_1+1
    INC
    STA text_row_2+1
    STA text_row_3+1
    INC
    STA text_row_4+1
    STA text_row_5+1
    INC
    STA text_row_6+1
    STA text_row_7+1
    LDA _bittext_x
    AND #$7F
    STA _bittext_x
    TYA
    ROR
    ROR
    AND #$80
    ORA _bittext_x
    STA _bittext_x
    RTS