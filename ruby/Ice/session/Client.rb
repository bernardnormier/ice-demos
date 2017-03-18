#!/usr/bin/env ruby
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# **********************************************************************

require 'Ice'
require 'thread'
require 'timeout'

Ice::loadSlice('Session.ice')

class Client < Ice::Application

    def initialize
        @mutex = Mutex.new
        @session = nil
    end

    def interruptCallback(sig)
        cleanup(true)
        begin
            Ice::Application::communicator.destroy
        rescue => ex
            puts ex
        end
        exit(0)
    end

    def run(args)
        if args.length > 0
            puts $0 + ": too many argumnets"
            return 1
        end

        #
        # Since this is an interactive demo we want the custom interrupt
        # callback to be called when the process is interrupted.
        #
        Ice::Application::callbackOnInterrupt

        while true
            print "Please enter your name ==> "
            STDOUT.flush
            name = STDIN.readline.chomp
            if name.length > 0
                break
            end
        end

        base = Ice::Application::communicator().propertyToProxy('SessionFactory.Proxy')
        factory = Demo::SessionFactoryPrx::checkedCast(base)
        if not factory
            puts $0 + ": invalid proxy"
            return 1
        end

        @mutex.synchronize {
            @session = factory.create(name)
        }

        begin

            hellos = []

            menu()

            destroy = true
            shutdown = false
            while true
                begin
                    print "==> "
                    STDOUT.flush
                    c = STDIN.readline.chomp
                    if c =~ /^[0-9]+$/
                        index = c.to_i
                        if index < hellos.length
                            hello = hellos[index]
                            hello.sayHello()
                        else
                            puts "Index is too high. " + hellos.length.to_s + " hello objects exist so far.\n" +\
                                 "Use `c' to create a new hello object."
                        end
                    elsif c == 'c'
                        hellos.push(@session.createHello())
                        puts "Created hello object " + (hellos.length - 1).to_s
                    elsif c == 's'
                        destroy = false
                        shutdown = true
                        break
                    elsif c == 'x'
                        break
                    elsif c == 't'
                        destroy = false
                        break
                    elsif c == '?'
                        menu()
                    else
                        puts "unknown command `" + c + "'"
                        menu()
                    end
                rescue EOFError
                    break
                end
            end

            cleanup(destroy)
            if shutdown
                factory.shutdown()
            end
        rescue
            begin
                cleanup(true)
            rescue
            end
        end

        return 0
    end

    def cleanup(destroy)
        @mutex.synchronize {
            if destroy && @session
                @session.destroy
            end
            @session = nil
        }
    end

    def menu
        print <<MENU
usage:
c:     create a new per-client hello object
0-9:   send a greeting to a hello object
s:     shutdown the server and exit
x:     exit
t:     exit without destroying the session
?:     help
MENU
    end
end

app = Client.new
exit(app.main(ARGV, "config.client"))
