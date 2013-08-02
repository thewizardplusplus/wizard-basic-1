	// создание переменных на стеке
	.text
	.global main
main:
	// выделяем память на стеке под две переменные
	enter $2, $0

	// загружаем число в формате с плавающей запятой на стек
	subl $4, %esp
	fld NUMBER
	fstps (%esp)

	// загружаем значение с вершины стека в первую переменную
	pop -4(%ebp)

	// загружаем число в формате с плавающей запятой на стек
	subl $4, %esp
	fld NUMBER
	fstps (%esp)

	// загружаем значение с вершины стека во вторую переменную 
	pop -8(%ebp)

	// загружаем значение из первой переменной на вершину стека
	push -4(%ebp)

	// загружаем значение из второй переменной на вершину стека
	push -8(%ebp)

	// вызываем функцию из библиотеки, очищаем стек от её параметров, кладём на стек её результат
	call NumberAdd
	addl $8, %esp
	subl $4, %esp
	fstps (%esp)

	// вызываем функцию из библиотеки, очищаем стек от её параметров, кладём на стек её результат
	call ArrayCreateFromNumber
	addl $4, %esp
	subl $4, %esp
	fstps (%esp)

	// вызываем функцию из библиотеки, очищаем стек от её параметров
	call Show
	add $4, %esp

	// освобождаем память, выделенную под переменные
	leave

	// выходим из программы, возвращая код успешного завершения
	mov $0, %eax
	ret

	.data
NUMBER:
	.float 2