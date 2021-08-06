/* Test machine: csel-kh4240-01.cselabs.umn.edu
* Name: Babacar Diouf, Salahudin Farah, Rezkath Awal
* X500: diouf006, farah176, awalx003 */

-Group ID on Canvas: 79

-How to compile and run our program:
	1. Open one terminal (for the server)
	2. Type "make clean" and hit the enter key
	3. Type "make web_server" and hit the enter key
	4. Type "./web_server <port> <path_to_testing>/testing <num_dispatch> <num_worker> 0 <queue_len> 0" and hit the enter key
	
	5. Open another terminal (for the client)
	6. Type "chmod +x web_server_sol"
	7. Then to send one request at at time, type "wget http://127.0.0.1:<port>/<image_or_text>/<if_image:gif_or_jpg_else_if_text_html_or_plain_or_big.txt>/<number>.<file_extension>" and hit the enter key
	8. To send multiple requests simultaneously, type "cat <path_to_urls_file> | xargs -n 1 -P <number_of_requests_at_a_time> wget" and hit the enter key
	
-Brief explanation on how your program works:  Our program takes a URL containing the path to a file the user wants to be downloaded.  The server takes that URL and downloads the desired file into a folder.

-Indicate which extra credit your group implements: None 

-Explanation of your policy to dynamically change the worker thread pool size: Not applicable. We did not implement Extra Credit A.

-Explanation of caching mechanism used: Not applicable.  We did not implement Extra Credit B.

-Contributions of each team member towards the project development:
	-Babacar Diouf
		-Helped write multiple functions
		-Helped debug the whole program
		-Helped with documentation
		-Helped write Interim Report for first submission deadline
	-Salahudin Farah
		-Helped write multiple functions
		-Helped debug the whole program
		-Helped with documentation
	-Rezkath Awal
		-Helped write multiple functions
		-Helped debug the whole program
		-Helped with documentation
		-Helped write Interim Report for first submission deadline
		-Wrote this README
		
		
		
		


		
		
