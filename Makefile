
all:
	cd dist && \
	cmake .. && make && \
	sudo mv libmysql-python.so.0.0.1 /usr/lib/mysql/plugin/py_eval.so && \
	sudo chmod -x /usr/lib/mysql/plugin/py_eval.so && \
	sudo chmod go-w /usr/lib/mysql/plugin/py_eval.so && \
	sudo chown root:root /usr/lib/mysql/plugin/py_eval.so
