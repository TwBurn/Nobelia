#!/usr/bin/env ruby
# coding: utf-8

# expecting 18900 stereo interleave 8

class Sector
  attr_accessor *%i(fileno channel submode coding data edc)
  def initialize fileno, channel, submode, coding, data, edc
    @fileno  = fileno
    @channel = channel
    @submode = submode
    @coding  = coding
    @data    = data
    @edc     = edc
  end
  def dump
    subheader = [@fileno, @channel, @submode, @coding].pack("CCCC")
    subheader * 2 + @data + [@edc].pack("N") # XXX: really edc in BE?
  end
  %i(eof realtime form2 trigger data audio video eor).reverse.each_with_index { |name,i|
    define_method(name){
      @submode & (1 << i)
    }
    define_method(:"#{name}="){ |value|
      @submode = (@submode & ~(1 << i)) | (value ? (1 << i) : 0)
    }
  }
  %i(emphasis bps sampfreq mono).reverse.each_with_index { |name,i|
    define_method(name){
      @coding & (3 << (i * 2))
    }
    define_method(:"#{name}="){ |value|
      @coding = (@coding & ~(3 << (i * 2))) | (value << (i * 2))
    }
  }
end

def main ofn, *acms
  acms = acms.each_with_index.map{|fn,i|
    File.open(fn, "rb"){|f|
      f.pos = 0x36;
      sectors = []
      while !f.eof
        sectors << Sector.new(1, i, 0x64, 0x05, f.read(2324), 0)
      end
      sectors
    }
  }

  interleaved = [Sector.new(1, 0, 0x60, 0x00, "\0" * 2324, 0)] * acms.map(&:size).max * 8

  acms.each_with_index { |sects,i|
    sects.each_with_index { |sect,j|
      interleaved[j * 8 + i] = sect
    }
  }

  (interleaved[-1] = interleaved[-1].dup).eof = true

  File.open(ofn, "wb") { |f|
    interleaved.each{|sect| f.write sect.dump }
  }
end

$0 == __FILE__ and main *ARGV
