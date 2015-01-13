
	###############	 #			    #	##############	  ###########
	             #   #              #   #				 #  		 #
				#    #				#   #				#             #
			   #     #				#   #				#
			  #		 #				#   #				#	
			 #		 #				#   #				 #
            #		 #				#   #				  #
           #		 #				#   ##############	   ##########   
		  #			 #				#   #							 #
		 #			 #				#   #							  #
		#			 #				#	#							   #
	   #			 # 			    #	#							   #
	  #				  #			   #	#				 #			  #
	 #				   #		  #		#				  # 		 #
	###############     ##########		##############     ##########


## BUILD

* configure
	To generate configuration file , Makefile and some C header files,you can use the shell script configure.

	Change the mode the configure file:

	```Bash
	chmod 755 ./configure
	```
	
	To see the option , you can use:
	
	```Bash
	./configure --help
	```

	To generate config file , Makefile and some C header files , you can use like this

	```Bash
	./configure --user=username --group=groupname --worker=n --port=m --resolution=t  \
				--log-dir=log --pid-dir=pid --conf-dir=conf
	```

* make

	```Bash
	make
	```

* install

	If you need create config directory/log directory/pid directory in some parent directory , 
	and you do not have the privilege , you need to use sudo.

	```Bash
	sudo make install
	```


## RUN

	The master process's user:group is user:user.
	The worker process's user:group is username:groupname according the config file.

	```Bash
	sudo ./zeus
	```
