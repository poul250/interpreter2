# interpreter2
## Interpreter of a model programming language.
This interpreter is a modification of https://github.com/poul250/interpreter.

### Formal grammar
* An entry of the form ⎨α⎬ means an iteration of the chain α, i.e. in the generated chain, either ε, α, αα, or ααα can be located in this place, etc.
* An entry of the form [α] means that either α or ε can be found in the desired chain at this point.
```
〈Program〉::= program {〈Descriptions〉〈Operators〉}
〈Descriptions〉 ::= ⎨〈Description〉;⎬
〈Description〉::=〈Type〉〈Variable〉⎨,〈Variable〉⎬
〈Type〉::= int | string | boolean | real
〈Variable〉::=〈identifier〉|〈Identifier〉=〈Constant〉
〈Constant〉::= 〈IntegerConstant〉|〈StringConstant〉|〈BoolConstant〉|〈RealConstant〉
〈IntegetConstant〉::= [〈Sign〉]〈Digit〉⎨〈Digit〉⎬
〈Sign〉::= + | -
〈StringConstant〉::= "⎨〈Literal〉⎬"
〈BoolConstant〉::= true | false
〈RealConstant〉::= [〈Sign〉]〈IntegerPart〉.〈FractionalPart〉
〈IntegerPart〉::=〈Digit〉⎨〈Digit〉⎬
〈FractionalPart〉::=〈Digit〉⎨〈Digit〉⎬
〈Operators〉::= ⎨〈Operator〉⎬
〈Operator〉::= if (〈Expression〉)〈Operator〉[ else〈Operator〉] |
                case (〈Expression〉) of〈CaseList〉end; |
                do〈Operator〉while (〈Expression〉); |
                while (〈Expression〉)〈Operator〉|
                for ( [〈Expression〉] ; [〈Expression〉] ; [〈Expression〉] )〈Operator〉|
                read (〈Identifier〉); |
                write (〈Expression〉⎨,〈Expression〉⎬); |
               〈CompositeOperator〉|
               〈Expression〉|
                continue; |
                break; |
〈CaseList〉::=〈Case〉⎨〈Case〉⎬
〈Case〉::=〈Constant〉⎨,〈Constant〉⎬ :〈Operator〉
〈CompositeOperator〉::= {〈Operators〉}
〈ExpressionOperator〉::=〈Expression〉

```

See specification.pdf for details
