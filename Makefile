server:	block_queue.h connection_pool.h http_conn.cpp http_conn.h locker.h log.cpp log.h lst_timer.h main.cpp threadpool.h
	g++ -o server block_queue.h connection_pool.h http_conn.cpp http_conn.h locker.h log.cpp log.h lst_timer.h main.cpp threadpool.h -lpthread -lmysqlclient

clean:
	rm -r server
