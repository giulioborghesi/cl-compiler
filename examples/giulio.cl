class Foo inherits IO {
    i : Object <- printh();
    k : Int <- 0;

    printh() : Object {
        {
        print();
        self;
        }
    };

    print() : Object {
        {
        out_int(k);
        out_string("\n");
        k <- k + 2;
        self;
        }
    };
};

class Bazz inherits Foo {
    j: Foo <- case self of 
        n : Bazz => new Foo;
        n : Foo => self;
    esac;
};

class Main {
  a : Bazz <- new Bazz;

  main() : Object {
      self
  };
};