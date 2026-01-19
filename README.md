Why?
--

I've spent too much time programming in Python. Python was my first "production"
language, and I never really stepped away from it in my career. I love Python, it's an
amazing language I'm more than willing to continue writing in. However, there was always this
weird mix of longing and shame. Longing for deeper understanding of how programs work at a lower level:
compilation, memory management, system calls, and such. And a shame for having these blanks after more than
15 years of doing programming professionally.

This project is my journey to finally fill some of the blanks. I'm still not sure why I picked C++ as the language.
I wanted to use C, but something swayed me towards its more horrific sibling. One thing I enjoy for sure is the sheer number
of ways to shoot yourself in the foot that stems from its ridiculous complexity that's been piling up over the years. The challenge
is worthwhile though - I find navigating this complexity immensely rewarding (yes, yes, with the help of LLMs). Journey before destination.

Timeline and steps
--

There are plenty of projects and tutorials on how to write a HTTP server "from scratch". They're all good as a starting point, but generally lack the
depth. I figured that the best way would be to follow my gut and experience in programming I've gained over the years. In this timeline I'm tracking the
steps I've taken so far in implementing the HTTP server.

1. Learning the language basics. C++ is a complex and huge language. To get the basics, I thoroughly worked my way through all chapters in the [learncpp tutorial](https://learncpp.com).
An awesome and free resource.

2. Before writing the server, I needed some basic understanding of how sockets and "stuff" work in Unix-like systems. The [Beej's network programming guide](https://beej.us/guide/bgnet/html/index-wide.html) was of great help and inspiration.

3. GitHub is full of "write-your-server" tutorials. I stumbled upon [this one - C-Web-Server](https://github.com/bloominstituteoftechnology/C-Web-Server), and it was
a good starting point. I ended up not following it all, though.

4. From the very start, I wanted to make my implementation somewhat compliant with the HTTP/1.1 standard. Meaning that I had to take request parsing seriously.
I was also eager to find test suites to use against my server. I must admit that for a brief moment of weakness I considered using an existing library for parsing requests.
But then, while looking at established HTTP server implementations on GitHub (mainly to see if I could reuse their test suites), I stumbled upon the [mongrel](https://github.com/mongrel/mongrel), a high-performance Ruby HTTP server. Not many tests I could've reused, but it had a weird requests parsing logic described in `*rl` files that looked awfully
similar to C. That's how I learned about [Ragel](https://www.colm.net/files/ragel/ragel-guide-6.10.pdf), and a way to implement an HTTP parser without losing too much dignity.
This [intro article](https://web.archive.org/web/20130307150514/http://zedshaw.com/essays/ragel_state_charts.html) about Ragel by Zed Shaw (the mongrel's author) was also helpful.

5. My first implementation of a server was a simple TCP server on blocking sockets that could accept basic HTTP requests and return a response compliant with the standard.

6. Then, I updated my server to serve static files from a `www` root. With that implementation, the testing became much more exciting - I generated a very heavy reveal.js presentation and attempted to serve it with my server to a standard browser. It instantly revealed humongous memory leaks and general slowness. At this point, I was comparing
the performance of my server with Python's standard `http.server` module. Needless to say that the Python server blew my masterpiece out of the water.

7. Somewhere around this time, I had started using `lldb` a lot. I had to learn it first - the learning curve wasn't horribly steep though. This debugger is a very advanced
and complex tool on its own, but the more I use it, the more confident I become in my ability to debug pretty much anything. I actually made a point of reading a book about `lldb`. Debugging a compiled language is very different from pdb/ipdb I employed a lot in Python. I'd also say that the earlier you get hands-on experience in debugging the better - for certain things like memory errors and leaks there is no alternative. I used the `leaks` program a bit to prove that my server had no memory leaks. Be prepared to learn a thing or
two about code signing in OS X.

8. Another tangential thing I had (and was willing) to learn was a build system for my project. Using `clang++` to compile one to two files was fun and dandy until it stopped.
CMake is a de-facto standard build system for C++ projects. Needless to say, a very complex and advanced system. It was a good investment of time though, considering that a lot
of projects use it and it did make adding dependencies to my project a breeze.

9. Having a somewhat working HTTP server capable of serving static files, I set the grand goal to beat the Python implementation in speed and memory footprint. Wow, right?

10. First thing was switching to non-blocking sockets and using `poll` to read requests and send responses. It made things faster.

11. Then, I implemented sending files with the `sendfile` system call. While it's not particularly cross-platform, it was faster and more memory efficient. Mainly it was faster and more memory efficient because I used a very naive approach previously - loading the entire file's content to memory even though I was sending it in chunks.

12. The lack of tests became real PITA. I was sheepishly postponing writing unit-test but I'm a man and could withstand only that much butthurt. I picked the Catch2 as my testing framework, and thanks to CMake it was super easy to add it as a dependency. Coming from Python and pytest worlds, at first it felt a bit clunky. But after initial "ugh", I now like it very much.

13. Everything is a stream. When you need to write or read things, treat them as a stream. I dumped the `sendfile` implementation after I'd learnt that it won't work over TLS (which I aspire to implement, maybe). Instead, I made all my response bodies to have a `std::istream` interface: read a chunk to a buffer, send it over, rinse and repeat. I was very pleased with the result, my implementation now had a smaller memory footprint compared to the Python implementation (what an achievement, to beat the interpreted language with GC, right?) and was as speedy too. Yay.

14. A natural next step was implementing HTTP requests pipelining. That is, when you might receive multiple requests in a single `recv` buffer or have to handle a mix of complete and incomplete requests that spans over several buffers. While it's an unlikely scenario on modern systems: the socket TCP buffers are quite large and you usually get an entire (and only one) request in a buffer, I wanted to make my parsing routines as solid as possible.

15. While unit-tests are useful, I wanted some end-to-end tests - verifying things manually in the browser is tedious. Here comes pytest again. As I learned, it's a standard practice in the world of compiled languages - using a more high-level language for writing tests. Many "famous" HTTP servers use Perl for running their tests. I didn't feel particularly brave that day, and chose the warmth and familiarity of Python.

16. Tried to play a large video from my server with an HTML5 player, and learned about content ranges in HTTP requests and responses. Surprisingly, with the response body content as `std::ostream`, it was trivial to implement support for ranges.

17. By this point, I was so happy with my server that I was contemplating ditching its development and moving on to something else. But then ...

18. YouTube Premium is expensive, especially when you're unemployed. And I can't stand ads. So, I was messing around with a small project to enable me watching the ad-free videos on a phone/in a browser. yt-dlp, wink-wink. The first implementation of the droxying (yes, not proxying - droxying; because proxying YT is illegal) server was in Python. To no one's surprise, Python was too memory-hungry to run it on a 512Mb DO droplet. I vibe-coded another implementation in Go - much better. But then, I figured - this is a perfect job for my amazing HTTP server!

19. While trying to fit the Python implementation into my tiny DO droplet, I profiled the memory usage of a program ... to start the interpreter itself costs ~120MB easily. Want a fancy async HTTP client? Simply importing `httpclient` without instantiating the actual client would cost you 30MB to 40MB. The price of convenience.

20. The first order of business was implementing the routing for requests. So I added an interface for request handlers, and some simple logic for mounting such a handler to an endpoint. Nothing fancy, probably with bugs, but works well enough.  

21. My mental model is still adjusting to the concept of resource ownership - if you need to keep objects between function calls, something must own them. Allocating things on stack won't automagically preserve them. C++ STL containers are quite handy here for they take away almost all complexity in allocating/deallocating objects. "Almost". I've lost count how many times I shot myself in the foot by not utilizing the move semantics correctly (or forgetting about it altogether). Also, some of container types INVALIDATE their members once resized (I'm talking about you, `std::vector`).

22. In Python, when you want to make a HTTP request, it's easy - use `urlopen`. Nah, who am I kidding - use `requests`, or something like `httpclient` for async stuff. And to proxy requests in my server, I'll need to make a lot of HTTP requests. All kinds of requests. For a brief moment of bravery, I entertained the thought of writing a bare-bones HTTP client from scratch. But then, since I'd be using it with real-world HTTP servers - "bare-bones" most likely won't do. Yet, I wanted to stay at a lowest level possible. Taking into account that my server implements is event-driven and non-blocking, the HTTP client must also be non-blocking. A prospect of using threads with some 3rd-party client didn't spark joy. The only low-level HTTP client that gives you control over when to drive it is `libcurl`. I was kinda excited about trying it out. I'm a frequent user of `curl`, I've always preferred it over more fancy tools, like Postman, for making requests when testing or debugging. While it has a learning curve, once you have fundamentals, it becomes a reliable tool.  

23. Two weeks later, I finally had a working non-blocking HTTP client based on libcurl. It was a journey that I thoroughly enjoyed. Only a few times, I broke down and cried in despair. One of the things I "enjoyed" the most was incorporating "foreign" sockets (owned by libcurl) into my simple poll-based event loop.

24. The importance of load testing. It was a smart decision to establish a suite of integration tests in Python. It was a piece of cake to implement a test case that spams the target server with hundreds of requests in dozens of threads. It was not a piece of cake to iron out all the bugs and design deficiencies this rate of requests revealed. Talking about my server supporting custom request handlers: for my load testing, I would spawn two processes of a server, where one server automatically mounts a handler that proxies all request to the other server. Having logs from both servers handy or running them with lldb was also instrumental in pinning down some of the bugs.

25. Something I've known all along from experience: there is no better debugging method for concurrent/async systems than writing logs in the right places and thinking hard. Also, when writing logs - make sure that all log records pertained to the same event (e.g., a HTTP request) have some unique identifier in them (e.g., a request id, a socket fd, whatever). It helps with tracing a lot. 

26. Implementing a simple async tasks queue was a lot of "fun". Learned a lot about forking, pipes, and signals. The fact that a child inherits signal handlers and open files from its parent was a surprise. Luckily, I caught it early in the unit tests - Catch2 was suspiciously receiving and reporting SIGTERM signals without test cases failing. As it turns out, every task I spawned for tests inherited signal handlers from the parent, including a SIGTERM handler from Catch2. When this task gets killed, it receives a SIGTERM from the parent, and it gets handled by Catch2's handlers that print it out as an error to stderr. It took many hours of debugging with Claude Code and then ChatGPT. ChatGPT proved itself much more useful in this situation.

27. In my "droxy" handler I had to implement proxying from a streaming endpoint. When everything is being driven by an event loop, sooner or later callbacks enter the stage. But I've always been hesitant about using callbacks unless absolutely necessary. While with libcurl it was the only option, in my request handlers I had options. After some very careful deliberation and multiple versions, I also settled on using callbacks to notify the server from within a handler about a response (or its chunk) ready to be written. It's not ideal, to be honest. But that's the best approach I was able to come up with while maintaining relatively simple logic. I thought hard about passing writer objects to handlers using which they could write whatever and whenever they want (which wouldn't result in an actual write, but rather in buffering bytes for sending when the event loop decides it's time). But I hit a wall in my design skills where everything I'd come up with was overly complicated and convoluted.

28. Compiling my server binary on modern Linux was a surprisingly low-effort endeavour. It even runs and does the "droxying", but nowhere near as good as the Go implementation. The memory and CPU footprints are smaller, though. 


... And that's it for now. I'm done with this project for the time being. It was an excellent exercise and great fun to implement. I like C++. 
