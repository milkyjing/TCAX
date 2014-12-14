
1. < key = value >, key is 'key', value is 'value'

2. <k  ey    =  value   >, key is 'k ey', value is 'value', Note, all spaces on the left or the right of the text will be discarded, all spaces between texts will be merged into a single space.

3. < "key" = value >, key is 'key', value is value

4. < " >key <=  " = "va""
lue">, key is ' >key <=  ', value is 'va"
lue', if you want to keep the spaces, and want to have the special markups as normal texts, use the quote mark.

5. <"ke""=value"=value>, looks strange, but this is valid, key is 'ke"=value', value is 'value'

invalid ones:

1. < k"ey" = value>

2. < ""key = value>

3. <"key = value">

if you want to use the "" markup, there should be one " between the markup < and the very first non-space text, and one " after the last non-space text and before the markup >

4. <"ke""y=""value""<="=value">, looks strange, but its key is valid, key is 'ke"y="value"<=', value is invalid
我艹艹
斯蒂芬
奥啊

<我 艹=fuck >

<<ke = sdf>  # <  will be ignored
<a<ke = sdf> # <a  will be ignored
<a=ke = sdf>  # invalid, the whole will be ignored
<a=<ke=sdf>   # <a=  will be ignored
<"a<d"=df">    # invalid
<"a<d"=df">">    # invalid
<"a<d"="df>" a>    # invalid
<"a<d"="df>" >    # OK
<"a<d"="df>" a">    # invalid
<"a<d"="df>"" a">    # OK




>>>>>. <<, dsf == <b=a>>>   # key = b, value = a
>>>>>. <<, dsf == <=b=a>>>   # invalid
>>>>>. <<, dsf == <=>b=a>>>   # invalid
>>>>>. <<, dsf == <"=>b"=     a   >>>   # key = =>b, value = a
>>>>>. <<, dsf == <"=>b"==     a   >>>   # invalid
>>>>>. <<, dsf == <"=>b"="=     a  " >>>   # key = =>b, value = '=     a  '

<
a
=
b
>    #invalid

<a
=
b
>    #invalid

<a
=b
>    #invalid


<a=b
>    #invalid

<a=b>    #OK

if you want to have a value which contains multiple lines, or the special markups such as < > = " etc, you have to use the quote mark


<"
a
"=b>    #OK

<"
a"
=b>    #invalid

<"
a
"="b
">    #OK


<
"a
"="bb
">    #invalid

<"a
"=
"bb
">    #invalid

<"a
"="bb
">    #OK

<"a
"="bb
"
>    #invalid

<"a
"="bb
">
>    #OK

<"<a
"="bb>
">
>    #OK

<a=""> # OK

<""="">  # OK, but meaningless


<""""="""">   #OK

<""""=""""">   #invalid

<"""""="""">   #invalid

<""""="""""">   #OK

<""""""="""">   #OK

<a=b><b=c><c=d>   # OK
<a=b<b=c><cd>   # b, c is OK

< a   bdsf d   f = """  b d   df">

< a   bdsf d   f =   b d   df>


<"""b d   df" = a>

