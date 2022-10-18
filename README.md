# What's this

This is a sandbox running on Linux. It's based on `seccomp`,`setrlimit` APIs.

With is you can run a guest executable and:

- Use seccomp rule to control guest's system calls
- Limit resources
  - Time(Both CPU Time& Real Time)
  - Memory
  - Output size(When stdout is redirected to file)
- Redirect stdin&stdout of guest
- Pass custom arguments and environment variable
- Logging basic information of guest

# Requirements

- Linux kernel 2.8+ (Confirmed OK on 5.10)

- GCC6+(Confirmed OK on 6.3.0)

# Installation

## Compile it yourself

`libseccomp` is required to compile

- On Debian&Ubuntu:

```bash
$ sudo apt-get install libseccomp-dev libseccomp2 seccomp
```

- On CentOS&RHEL:

```bash
$ sudo yum -y install libseccomp
```

- Or you can compile from source

  Go to  [seccomp/libseccomp](https://github.com/seccomp/libseccomp) for information

Clone this repo

```bash
$ git clone 
```

Make and install it (to `/usr/bin/`)

```bash
$ cd sandbox
$ make
$ sudo make install
```

Now you can run it

```bash
$ sandbox
```

# Example

It's recommended to pass arguments using quotes to avoid some special characters in string like `=`,`;`

```
$ sudo sandbox -p"./tests/helloworld" -a"1,2,3,4" -e"USER=bakaft;VERSION=1.0"
$ sudo sandbox --exe_path=./tests/helloworld --exe_args=1,2,3,4 --exe_envs="USER=bakaft;VERSION=1.0"
```

# Credits

Thanks to these projects for inspiration

[QingdaoU/Judger](https://github.com/QingdaoU/Judger)

[SDUOJ/sduoj-sandbox](https://github.com/SDUOJ/sduoj-sandbox)

Thanks to these projects for providing useful codes

[yksz/c-logger](https://github.com/yksz/c-logger)
