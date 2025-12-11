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

2. Before writing the server, I needed some basic understanding of how sockets and "stuff" work in Unix-like system. The [Beej's network programming guide](https://beej.us/guide/bgnet/html/index-wide.html) was of great help and inspiration.

3. GitHub is full of "write-your-server" tutorials. I stumbled upon [this one - C-Web-Server](https://github.com/bloominstituteoftechnology/C-Web-Server), and it was
a good starting point. I ended up not following it all, though.

4. From the very start, I wanted to make my implementation somewhat compliant with the HTTP/1.1 standard. Meaning that I had to take request parsing seriously.
I was also eager to find test suites to use against my server. I must admit, that for a brief moment of weakness I considered using an existing library for parsing requests.
But then, while looking at established HTTP server implementations on GitHub (mainly to see if I could reuse their test suites), I stumbled upon the [mongrel](https://github.com/mongrel/mongrel/), a high-performant Ruby HTTP server. Not many tests I could've reused, but it had a weird requests parsing logic described in `*rl` files that looked awfully
similar to C. That's how I learned about [Ragel](https://www.colm.net/files/ragel/ragel-guide-6.10.pdf), and a way to implement a HTTP parser without losing too much dignity.
This [intro article](https://web.archive.org/web/20130307150514/http://zedshaw.com/essays/ragel_state_charts.html) about Ragel by Zed Shaw (the mongrel's author) was also helpful.

5. My first implementation of a server was a simple TCP server on blocking sockets that could accept basic HTTP requests and return a response compliant with the standard.

6. Then, I updated my server to serve static files from a `www` root. With that implementation, the testing became much more exciting - I generated a very heavy reveal.js presentation and attempted to serve it with my server to a standard browser. It instantly revealed humongous memory leaks and general slowness. At this point, I was comparing
the performance of my server with the standard Python's `http.server` module. Needless to say that the Python server blew my masterpiece out of the water.

7. Somewhere around this time, I had started using `lldb` a lot. I had to learn it first - the learning curve wasn't horribly steep though. This debugger is a very advanced
and complex tool on its own, but the more I use it, the more confident I become in my ability to debug pretty much anything. I actually made a point of reading a book about `lldb`. Debugging a compiled language is very different from pdb/ipdb I employed a lot in Python. I'd also say that the earlier you get hands on experience in debugging the better - for certain things like memory errors and leaks there is no alternative. I used the `leaks` program a bit to prove that my server had no memory leaks. Be prepared to learn a thing or
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
