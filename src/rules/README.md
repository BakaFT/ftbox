There are some rules you must consider

- never allow `alarm(0)`

  the sandbox use `setitimer` to set time limit, which could be canceled by calling `alarm(0)`

- never allow `open` with `O_WRONLY`or`O_RDWR`

  this could be dangerous if you allow guest to write 

- `openat` is called in `open`, don't forget this , do what you did with `open` to `openat`
