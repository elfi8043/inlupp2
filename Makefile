CC = gcc
FLAGS = -Wall -Wextra -g
LIB = db_backend.c utils.c hash_table.c linked_list.c list_iterator.c

frontend: frontend.c $(LIB)
	$(CC) $(FLAGS) frontend.c $(LIB) -o frontend

run: frontend
	./frontend < webstoretest.txt

memrun: frontend
	valgrind --leak-check=full ./frontend < webstoretest.txt

db_tests: db_tests.c $(LIB)
	$(CC) $(FLAGS) db_tests.c $(LIB) -lcunit -o db_tests

test: db_tests
	./db_tests

memtest: db_tests
	valgrind --leak-check=full ./db_tests

db_coverage: db_tests.c $(LIB)
	$(CC) $(FLAGS) --coverage db_tests.c $(LIB) -lcunit -o db_coverage

coverage: db_coverage
	rm -f *.gcda
	./db_coverage
	gcovr --filter db_backend.c
	gcovr --txt-metric branch --filter db_backend.c
	gcovr --html-details db_coverage.html --filter db_backend.c

clean:
	rm -f frontend db_tests db_coverage
	rm -f *.gcda *.gcno *.gcov db_coverage*.html db_coverage*.css
	rm -rf cov