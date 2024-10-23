# Compilador da Linguagem C- (C menos)

## Léxico

Nessa etapa ocorre o reconhecimento de tokens. Para a realização dessa tarefa, temos dois padrões auxiliares:

- CHAR: Qualquer caractere, maiúsculo ou minúsculo entre A e Z
- DIGIT: Qualquer digito entre 0 e 9

Abaixo contém uma lista de símbolos reconhecidos pelo compilador. O nome antes da descrição do padrão de simbolos é o token atribuído quando o padrão é encontrado

- CSYMBOL: Qualquer um dos seguintes símbolos: \+ \- \* \/ \< \> \= \; \, \( \) \[ \] \{ \}
- ID: Qualquer palavra formada por um ou mais CSYMBOL
- NUM: Qualquer número formado por um ou mais DIGIT
- COMMENT: Qualquer texto escrito entre os símbolos "/\*" e "\*/" (Com exceção do caractere "/")
- SYMBOL: Representa os simbolos "<=", ">=", "==", "!="

A lista restante dos tokens representam palavras chaves e são identificados pelo próprio nome:

- IF, ELSE, INT, RETURN, VOID, WHILE.

Cada token reconhecido é disparado para o analisador sintático, antes mesmo do analisador léxico começar a analisar o próximo padrão.
