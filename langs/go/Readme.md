Enviroment Settings
===================

    export GOPATH=.
    export PATH=bin:$PATH

    cd src/github.com/maxint/string
    go build
    go install
    cd -
    cd src/github.com/maxint/hello
    go build
    go install
    cd -
    hello # test
