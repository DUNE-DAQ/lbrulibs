import h5py
import sys
import numpy as np
import struct

#-----------------------------------------------------------------------------
# Packet type section
#-----------------------------------------------------------------------------
#Key
class Key(object):
    key_delimiter = '-'
    key_format = key_delimiter.join(('{io_group}', '{io_channel}', '{chip_id}'))

    def __init__(self, *args):
        self._initialized = False
        if len(args) == 3:
            self.io_group = args[0]
            self.io_channel = args[1]
            self.chip_id = args[2]
        elif len(args) == 1:
            if isinstance(args[0], Key):
                self.io_group = args[0].io_group
                self.io_channel = args[0].io_channel
                self.chip_id = args[0].chip_id
            elif isinstance(args[0], bytes):
                self.keystring = str(args[0].decode("utf-8"))
            else:
                self.keystring = str(args[0])
        else:
            raise TypeError('Key() takes 1 or 3 arguments ({} given)'.format(len(args)))
        self._initialized = True

    def __repr__(self):
        return 'Key(\'{}\')'.format(self.keystring)

    def __str__(self):
        return self.keystring

    def __eq__(self, other):
        if isinstance(other, Key):
            return self.io_group == other.io_group and \
            self.io_channel == other.io_channel \
            and self.chip_id == other.chip_id
        if isinstance(other, tuple):
            return self.io_group == other[0] and self.io_channel == other[1] \
            and self.chip_id == other[2]
        if str(self) == str(other):
            return True
        return False

    def __ne__(self, other):
        return not self == other

    def __hash__(self):
        return hash(str(self))

    def __getitem__(self, index):
        return (self.io_group, self.io_channel, self.chip_id)[index]

    @property
    def keystring(self):
        return Key.key_format.format(
                io_group = self.io_group,
                io_channel = self.io_channel,
                chip_id = self.chip_id
            )

    @keystring.setter
    def keystring(self, val):
        if self._initialized:
            raise AttributeError('keystring cannot be modified')
        if not isinstance(val, str):
            raise TypeError('keystring must be str')
        parsed_keystring = val.split(Key.key_delimiter)
        if len(parsed_keystring) != 3:
            raise ValueError('invalid keystring formatting')
        self.io_group = int(parsed_keystring[0])
        self.io_channel = int(parsed_keystring[1])
        self.chip_id = int(parsed_keystring[2])

    @property
    def chip_id(self):
        return self._chip_id

    @chip_id.setter
    def chip_id(self, val):
        if self._initialized:
            raise AttributeError('chipid cannot be modified')
        chip_id = int(val)
        if chip_id > 255 or chip_id < 0:
            raise ValueError('chip_id must be 1-byte ({} invalid)'.format(chip_id))
        self._chip_id = chip_id

    @property
    def io_channel(self):
        return self._io_channel

    @io_channel.setter
    def io_channel(self, val):
        if self._initialized:
            raise AttributeError('io_channel cannot be modified')
        io_channel = int(val)
        if io_channel > 255 or io_channel < 0:
            raise ValueError('io_channel must be 1-byte ({} invalid)'.format(io_channel))
        self._io_channel = io_channel

    @property
    def io_group(self):
        return self._io_group

    @io_group.setter
    def io_group(self, val):
        if self._initialized:
            raise AttributeError('io_group cannot be modified')
        io_group = int(val)
        if io_group > 255 or io_group < 0:
            raise ValueError('io_group must be 1-byte ({} invalid)'.format(io_group))
        self._io_group = io_group

    @staticmethod
    def is_valid_keystring(keystring):
        if not isinstance(keystring, str):
            return False
        try:
            key = Key(keystring)
        except ValueError:
            return False
        return True

    def to_dict(self):
        return_dict = dict(
                io_group = self.io_group,
                io_channel = self.io_channel,
                chip_id = self.chip_id
            )
        return return_dict

    @staticmethod
    def from_dict(d):
        req_keys = ('io_group', 'io_channel', 'chip_id')
        if not all([key in d for key in req_keys]):
            raise ValueError('dict must specify {}'.format(req_keys))
        return Key(d['io_group'], d['io_channel'], d['chip_id'])

from bitarray import bitarray

def fromuint(val, nbits, endian='big'):
    try:
        if isinstance(nbits, slice):
            nbits = abs(nbits.stop - nbits.start)
        if endian[0] == 'b':
            string = bin(val)[2:].zfill(nbits)
            return bitarray(string)
        string = bin(val)[-1:1:-1].ljust(nbits,'0')
        return bitarray(string)
    except TypeError:
        return val

def touint(bits, endian='big'):
    bin_string = bits.to01()
    if endian[0] == 'b':
        return int(bin_string, 2)
    return int(bin_string[::-1], 2)

#PacketV2
def _clears_cached_int(func):
    def new_func(self, *args, **kwargs):
        self._int = None
        return func(self, *args, **kwargs)
    return new_func

def _clears_cached_chip_key(func):
    def new_func(self, *args, **kwargs):
        if hasattr(self, '_chip_key'):
            del self._chip_key
        return func(self,*args,**kwargs)
    return new_func

class Packet_v2(object):
    asic_version = 2
    size = 64
    num_bytes = 8

    # shared by all packet types
    packet_type_bits = slice(0,2)
    chip_id_bits = slice(2,10)
    downstream_marker_bits = slice(62,63)
    parity_bits = slice(63,64)
    parity_calc_bits = slice(0,63)

    # only data packets
    channel_id_bits = slice(10,16)
    timestamp_bits = slice(16,47)
    first_packet_bits = slice(47,48)
    dataword_bits = slice(48,56)
    trigger_type_bits = slice(56,58)
    local_fifo_bits = slice(58,60)
    shared_fifo_bits = slice(60,62)
    # only if fifo diagnostics enabled
    fifo_diagnostics_timestamp_bits = slice(16,32)
    local_fifo_events_bits = slice(44,46)
    shared_fifo_events_bits = slice(32,44)

    # only read/write packets
    register_address_bits = slice(10,18)
    register_data_bits = slice(18,26)

    fifo_diagnostics_enabled = False

    DATA_PACKET = 0
    TEST_PACKET = 1
    CONFIG_WRITE_PACKET = 2
    CONFIG_READ_PACKET = 3

    NORMAL_TRIG = 0
    EXT_TRIG = 1
    CROSS_TRIG = 2
    PERIODIC_TRIG = 3

    endian = 'little'

    def __init__(self, bytestream=None):
        self._int = None
        if bytestream is None:
            self.bits = bitarray(self.size,endian=self.endian)
            self.bits.setall(False)
            return
        elif len(bytestream) == self.num_bytes:
            self.bits = bitarray(endian=self.endian)
            self.bits.frombytes(bytestream)
        else:
            raise ValueError('Invalid number of bytes: %s' %
                    len(bytestream))

    def __eq__(self, other):
        return self.bits == other.bits

    def __ne__(self, other):
        return not (self == other)

    def __str__(self):
        strings = []
        if hasattr(self, 'direction'):
            strings += ['Direction: {}'.format(self.direction)]
        strings += ['Key: {}'.format(self.chip_key)]
        strings += ['Chip: {}'.format(self.chip_id)]
        if self.downstream_marker:
            strings += ['Downstream']
        else:
            strings += ['Upstream']
        if self.packet_type == self.DATA_PACKET:
            strings += ['Data']
            strings += ['Channel: {}'.format(self.channel_id)]
            strings += ['Timestamp: {}'.format(self.timestamp)]
            strings += ['First packet: {}'.format(self.first_packet)]
            strings += ['Dataword: {}'.format(self.dataword)]
            strings += ['Trigger: {}'.format({
                self.NORMAL_TRIG: 'normal',
                self.EXT_TRIG: 'external',
                self.CROSS_TRIG: 'cross',
                self.PERIODIC_TRIG: 'periodic'
                }[self.trigger_type])]
            if self.local_fifo_full:
                strings += ['Local FIFO 100%']
            elif self.local_fifo_half:
                strings += ['Local FIFO >50%']
            else:
                strings += ['Local FIFO ok']
            if self.shared_fifo_full:
                strings += ['Shared FIFO 100%']
            elif self.shared_fifo_half:
                strings += ['Shared FIFO >50%']
            else:
                strings += ['Shared FIFO ok']
            if self.fifo_diagnostics_enabled:
                strings += ['Local FIFO: {}'.format(self.local_fifo_events)]
                strings += ['Shared FIFO: {}'.format(self.shared_fifo_events)]
        elif self.packet_type == self.TEST_PACKET:
            strings += ['Test']
        elif self.packet_type in (self.CONFIG_READ_PACKET, self.CONFIG_WRITE_PACKET):
            strings += [{
                self.CONFIG_READ_PACKET: 'Read',
                self.CONFIG_WRITE_PACKET: 'Write'
            }[self.packet_type]]
            strings += ['Register: {}'.format(self.register_address)]
            strings += ['Value: {}'.format(self.register_data)]

        if hasattr(self, 'receipt_timestamp'):
            strings += ['Receipt TS: {}'.format(self.receipt_timestamp)]
        strings += ['Parity: {} (valid: {})'.format(self.parity,
            self.has_valid_parity())]
        return '[ ' + ' | '.join(strings) + ' ]'

    def __repr__(self):
        return 'Packet_v2(' + str(self.bytes()) + ')'

    def bytes(self):
        return self.bits.tobytes() #[::-1]

    def export(self):
        type_map = {
                self.TEST_PACKET: 'test',
                self.DATA_PACKET: 'data',
                self.CONFIG_WRITE_PACKET: 'config write',
                self.CONFIG_READ_PACKET: 'config read'
                }
        d = {}
        d['asic_version'] = self.asic_version
        d['chip_key'] = str(self.chip_key) if self.chip_key else None
        d['io_group'] = self.io_group
        d['io_channel'] = self.io_channel
        d['bits'] = self.bits.to01()
        d['type_str'] = type_map[self.packet_type]
        d['packet_type'] = self.packet_type
        d['chip_id'] = self.chip_id
        d['downstream_marker'] = self.downstream_marker
        d['parity'] = self.parity
        d['valid_parity'] = self.has_valid_parity()
        ptype = self.packet_type
        if ptype == self.TEST_PACKET:
            pass
        elif ptype == self.DATA_PACKET:
            d['channel_id'] = self.channel_id
            d['timestamp'] = self.timestamp
            d['first_packet'] = self.first_packet
            d['dataword'] = self.dataword
            d['trigger_type'] = self.trigger_type
            d['local_fifo'] = self.local_fifo
            d['shared_fifo'] = self.shared_fifo
            if self.fifo_diagnostics_enabled:
                d['local_fifo_events'] = self.local_fifo_events
                d['shared_fifo_events'] = self.shared_fifo_events
        elif (ptype == self.CONFIG_READ_PACKET or ptype ==
                self.CONFIG_WRITE_PACKET):
            d['register_address'] = self.register_address
            d['register_data'] = self.register_data
        if hasattr(self,'direction'):
            d['direction'] = self.direction
        if hasattr(self,'receipt_timestamp'):
            d['receipt_timestamp'] = self.receipt_timestamp
        return d

    def from_dict(self, d):
        if not d['asic_version'] == self.asic_version:
            raise ValueError('invalid asic version {}'.format(d['asic_version']))
        if 'type' in d and d['type'] not in (self.DATA_PACKET, self.TEST_PACKET, self.CONFIG_WRITE_PACKET, self.CONFIG_READ_PACKET):
            raise ValueError('invalid packet type for Packet_v2')
        if 'local_fifo_events' in d or 'shared_fifo_events' in d:
            self.fifo_diagnostics_enabled = True
        for key, value in d.items():
            if key in ('type_str', 'valid_parity'):
                continue
            elif key == 'bits':
                self.bits = bitarray(value)
            else:
                setattr(self, key, value)

    def as_int(self):
        if self._int is None:
            self._int = touint(self.bits, endian=self.endian)
        return self._int

    @property
    def chip_key(self):
        if hasattr(self, '_chip_key'):
            return self._chip_key
        if self.io_group is None or self.io_channel is None:
            return None
        self._chip_key = Key(self.io_group, self.io_channel, self.chip_id)
        return self._chip_key

    @chip_key.setter
    @_clears_cached_int
    @_clears_cached_chip_key
    def chip_key(self, value):
        if value is None:
            self.io_channel = None
            self.io_group = None
            return
        if isinstance(value, Key):
            self.io_group = value.io_group
            self.io_channel = value.io_channel
            self.chip_id = value.chip_id
            return
        self.chip_key = Key(value)

    @property
    def io_group(self):
        if hasattr(self, '_io_group'):
            return self._io_group
        return None

    @io_group.setter
    @_clears_cached_chip_key
    def io_group(self, value):
        if value is None:
            if hasattr(self, '_io_group'):
                del self._io_group
            return
        self._io_group = value

    @property
    def io_channel(self):
        if hasattr(self, '_io_channel'):
            return self._io_channel
        return None

    @io_channel.setter
    @_clears_cached_chip_key
    def io_channel(self, value):
        if value is None:
            if hasattr(self, '_io_channel'):
                del self._io_channel
            return
        self._io_channel = value

    @property
    def timestamp(self):
        if self.fifo_diagnostics_enabled:
            return touint(self.bits[self.fifo_diagnostics_timestamp_bits], endian=self.endian)
        return touint(self.bits[self.timestamp_bits], endian=self.endian)

    @timestamp.setter
    @_clears_cached_int
    def timestamp(self, value):
        if self.fifo_diagnostics_enabled:
            self.bits[self.fifo_diagnostics_timestamp_bits] = fromuint(value, self.fifo_diagnostics_timestamp_bits, endian=self.endian)
        else:
            self.bits[self.timestamp_bits] = fromuint(value, self.timestamp_bits, endian=self.endian)

    @property
    def local_fifo_half(self):
        return self.local_fifo%2

    @local_fifo_half.setter
    @_clears_cached_int
    def local_fifo_half(self, value):
        self.local_fifo = self.local_fifo_full*2 + value

    @property
    def local_fifo_full(self):
        return self.local_fifo//2

    @local_fifo_full.setter
    @_clears_cached_int
    def local_fifo_full(self, value):
        self.local_fifo = value*2 + self.local_fifo_half

    @property
    def shared_fifo_half(self):
        return self.shared_fifo%2

    @shared_fifo_half.setter
    @_clears_cached_int
    def shared_fifo_half(self, value):
        self.shared_fifo = self.shared_fifo_full*2 + value

    @property
    def shared_fifo_full(self):
        return self.shared_fifo//2

    @shared_fifo_full.setter
    @_clears_cached_int
    def shared_fifo_full(self, value):
        self.shared_fifo = value*2 + self.shared_fifo_half

    def compute_parity(self):
        return 1 - (self.bits[self.parity_calc_bits].count(True) % 2)

    @_clears_cached_int
    def assign_parity(self):
        self.parity = self.compute_parity()

    def has_valid_parity(self):
        return self.parity == self.compute_parity()

    @property
    def local_fifo_events(self):
        if self.fifo_diagnostics_enabled:
            bit_slice = self.local_fifo_events_bits
            return touint(self.bits[bit_slice], endian=self.endian)
        return None

    @local_fifo_events.setter
    @_clears_cached_int
    def local_fifo_events(self, value):
        if self.fifo_diagnostics_enabled:
            bit_slice = self.local_fifo_events_bits
            self.bits[bit_slice] = fromuint(value, bit_slice, endian=self.endian)

    @property
    def shared_fifo_events(self):
        if self.fifo_diagnostics_enabled:
            bit_slice = self.shared_fifo_events_bits
            return touint(self.bits[bit_slice], endian=self.endian)
        return None

    @shared_fifo_events.setter
    @_clears_cached_int
    def shared_fifo_events(self, value):
        if self.fifo_diagnostics_enabled:
            bit_slice = self.shared_fifo_events_bits
            self.bits[bit_slice] = fromuint(value, bit_slice, endian=self.endian)

    @property
    def chip_id(self):
        bit_slice = self.chip_id_bits
        return touint(self.bits[bit_slice], endian=self.endian)

    @chip_id.setter
    @_clears_cached_int
    @_clears_cached_chip_key
    def chip_id(self, value):
        bit_slice = self.chip_id_bits
        self.bits[bit_slice] = fromuint(value, bit_slice, endian=self.endian)

    @classmethod
    def _basic_getter(cls, name):
        bit_slice = getattr(cls, name + '_bits')
        mask = (~(((2**cls.size)-1 << (bit_slice.stop-bit_slice.start))) & (2**cls.size)-1)
        def basic_getter_func(self):
            return (self.as_int() >> bit_slice.start) & mask
        return basic_getter_func

    @classmethod
    def _basic_setter(cls, name):
        bit_slice = getattr(cls, name + '_bits')
        @_clears_cached_int
        def basic_setter_func(self, value):
            self.bits[bit_slice] = fromuint(value, bit_slice, endian=self.endian)
        return basic_setter_func

Packet_v2.packet_type = property(Packet_v2._basic_getter('packet_type'),Packet_v2._basic_setter('packet_type'))
Packet_v2.downstream_marker = property(Packet_v2._basic_getter('downstream_marker'),Packet_v2._basic_setter('downstream_marker'))
Packet_v2.parity = property(Packet_v2._basic_getter('parity'),Packet_v2._basic_setter('parity'))
Packet_v2.channel_id = property(Packet_v2._basic_getter('channel_id'),Packet_v2._basic_setter('channel_id'))
Packet_v2.dataword = property(Packet_v2._basic_getter('dataword'),Packet_v2._basic_setter('dataword'))
Packet_v2.first_packet = property(Packet_v2._basic_getter('first_packet'),Packet_v2._basic_setter('first_packet'))
Packet_v2.trigger_type = property(Packet_v2._basic_getter('trigger_type'),Packet_v2._basic_setter('trigger_type'))
Packet_v2.register_address = property(Packet_v2._basic_getter('register_address'),Packet_v2._basic_setter('register_address'))
Packet_v2.register_data = property(Packet_v2._basic_getter('register_data'),Packet_v2._basic_setter('register_data'))
Packet_v2.local_fifo = property(Packet_v2._basic_getter('local_fifo'),Packet_v2._basic_setter('local_fifo'))
Packet_v2.shared_fifo = property(Packet_v2._basic_getter('shared_fifo'),Packet_v2._basic_setter('shared_fifo'))

# Trigger packet
class TriggerPacket(object):
    packet_type = 7
    
    def __init__(self, trigger_type=None, timestamp=None, io_group=None):
        self.trigger_type = trigger_type
        self.timestamp = timestamp
        self.io_group = io_group

    def __eq__(self, other):
        if self.trigger_type != other.trigger_type: return False
        if self.timestamp != other.timestamp: return False
        if self.io_group != other.io_group: return False
        return True

    def __ne__(self, other):
        return not (self == other)

    def __str__(self):
        strings = ['Trigger']
        if self.io_group is not None:
            strings.append('IO group: {}'.format(self.io_group))        
        if self.trigger_type is not None:
            strings.append('Type: {}'.format(self.trigger_type))
        if self.timestamp is not None:
            strings.append('Timestamp: {}'.format(self.timestamp))
        return '[ '+' | '.join(strings)+' ]'

    def __repr__(self):
        strings = list()
        if self.io_group is not None:
            strings.append('io_group={}'.format(self.io_group))        
        if self.trigger_type is not None:
            strings.append('trigger_type={}'.format(self.trigger_type))
        if self.timestamp is not None:
            strings.append('timestamp={}'.format(self.timestamp))
        return 'TriggerPacket('+', '.join(strings)+')'

    def export(self):
        d = dict()
        d['io_group'] = self.io_group
        d['trigger_type'] = self.trigger_type
        d['timestamp'] = self.timestamp
        d['type'] = self.packet_type
        return d

    def from_dict(self, d):
        if 'io_group' in d:
            self.io_group = d['io_group']
        if 'trigger_type' in d:
            self.trigger_type = d['trigger_type']
        if 'timestamp' in d:
            self.timestamp = d['timestamp']

    @property
    def chip_key(self):
        if self.io_group:
            return Key(self.io_group,0,0)
        return None

    @chip_key.setter
    def	chip_key(self, val):
        key = Key(val)
        self.io_group = key.io_group

# Sync packet
from collections import defaultdict
class SyncPacket(object):
    packet_type = 6
    
    pretty_sync_type = defaultdict(lambda:'OTHER')
    pretty_sync_type[b'S'] = 'SYNC'
    pretty_sync_type[b'H'] = 'HEARTBEAT'
    pretty_sync_type[b'C'] = 'CLOCK SWITCH'
    
    def __init__(self, sync_type=None, clk_source=None, timestamp=None, io_group=None):
        self.sync_type = sync_type
        self.clk_source = clk_source
        self.timestamp = timestamp
        self.io_group = io_group

    def __eq__(self, other):
        if self.sync_type != other.sync_type: return False
        if self.clk_source != other.clk_source: return False
        if self.timestamp != other.timestamp: return False
        if self.io_group != other.io_group: return False
        return True

    def __ne__(self, other):
        return not (self == other)

    def __str__(self):
        strings = ['Sync']
        if self.io_group is not None:
            strings.append('IO group: {}'.format(self.io_group))        
        if self.sync_type is not None:
            strings.append('Type: {}'.format(self.pretty_sync_type[self.sync_type]))
        if self.timestamp is not None:
            strings.append('Timestamp: {}'.format(self.timestamp))
        if self.clk_source is not None:
            strings.append('Clk source: {}'.format(self.clk_source))
        return '[ '+' | '.join(strings)+' ]'

    def __repr__(self):
        strings = list()
        if self.io_group is not None:
            strings.append('io_group={}'.format(self.io_group))        
        if self.sync_type is not None:
            strings.append('sync_type={}'.format(self.sync_type))
        if self.timestamp is not None:
            strings.append('timestamp={}'.format(self.timestamp))
        if self.clk_source is not None:
            strings.append('clk_source={}'.format(self.clk_source))
        return 'SyncPacket('+', '.join(strings)+')'

    def export(self):
        d = dict()
        d['io_group'] = self.io_group
        d['sync_type'] = self.sync_type
        d['timestamp'] = self.timestamp
        d['clk_source'] = self.clk_source
        d['type'] = self.packet_type
        return d

    def from_dict(self, d):
        if 'io_group' in d:
            self.io_group = d['io_group']
        if 'sync_type' in d:
            self.sync_type = d['sync_type']
        if 'timestamp' in d:
            self.timestamp = d['timestamp']
        if 'clk_source' in d:
            self.clk_source = d['clk_source']

    @property
    def chip_key(self):
        if self.io_group:
            return Key(self.io_group,0,0)
        return None

    @chip_key.setter
    def chip_key(self, val):
        key = Key(val)
        self.io_group = key.io_group

#Timestamp packet
class TimestampPacket(object):
    size = 56
    chip_key = None
    def __init__(self, timestamp=None, code=None):
        self.packet_type = 4
        if code:
            self.timestamp = struct.unpack('<Q', code + b'\x00')[0]
        else:
            self.timestamp = timestamp

    def __str__(self):
        return '[ Timestamp: %d ]' % self.timestamp

    def __repr__(self):
        return 'TimestampPacket(%d)' % self.timestamp

    def __eq__(self, other):
        return self.timestamp == other.timestamp

    def __ne__(self, other):
        return not (self == other)

    def export(self):
        return {
                'type_str': 'timestamp',
                'type': self.packet_type,
                'timestamp': self.timestamp,
                'bits': self.bits.to01(),
                }

    def from_dict(self, d):
        if 'type' in d and d['type'] != self.packet_type:
            raise ValueError('invalid packet type for TimestampPacket')
        for key, value in d.items():
            if key == 'type':
                self.packet_type = value
            elif key == 'type_str':
                continue
            elif key == 'bits':
                self.bits = bitarray(value)
            else:
                setattr(self, key, value)

    @property
    def bits(self):
        return fromuint(self.timestamp, self.size)

    @bits.setter
    def bits(self, value):
        self.timestamp = touint(value)

    def bytes(self):
        return struct.pack('Q', self.timestamp)

# PacketV1
class Packet_v1(object):
    asic_version = 1
    size = 54
    num_bytes = 7
    packet_type_bits = slice(52, 54)
    chipid_bits = slice(44, 52)
    parity_bit = 0
    parity_calc_bits = slice(1, 54)
    channel_id_bits = slice(37, 44)
    timestamp_bits = slice(13, 37)
    dataword_bits = slice(3, 13)
    fifo_half_bit = 2
    fifo_full_bit = 1
    register_address_bits = slice(36, 44)
    register_data_bits = slice(28, 36)
    config_unused_bits = slice(1, 28)
    test_counter_bits_11_0 = slice(1, 13)
    test_counter_bits_15_12 = slice(40, 44)

    DATA_PACKET = bitarray('00')
    TEST_PACKET = bitarray('01')
    CONFIG_WRITE_PACKET = bitarray('10')
    CONFIG_READ_PACKET = bitarray('11')
    _bit_padding = bitarray('00')

    def __init__(self, bytestream=None):
        if bytestream is None:
            self.bits = bitarray(self.size)
            self.bits.setall(False)
            return
        elif len(bytestream) == self.num_bytes:
            reversed_bytestream = bytestream[::-1]
            self.bits = bitarray()
            self.bits.frombytes(reversed_bytestream)
            self.bits = self.bits[2:]
        else:
            raise ValueError('Invalid number of bytes: %s' %
                    len(bytestream))

    def __eq__(self, other):
        return self.bits == other.bits

    def __ne__(self, other):
        return not (self == other)

    def __str__(self):
        string = '[ '
        string += 'Chip key: {} | '.format(self.chip_key)
        if hasattr(self, 'direction'):
            string += 'Direction: {}'.format(self.direction)
            string += ' | '
        ptype = self.packet_type
        if ptype == Packet_v1.TEST_PACKET:
            string += 'Test | '
            string += 'Counter: %d | ' % self.test_counter
        elif ptype == Packet_v1.DATA_PACKET:
            string += 'Data | '
            string += 'Channel: %d | ' % self.channel_id
            string += 'Timestamp: %d | ' % self.timestamp
            string += 'ADC data: %d | ' % self.dataword
            string += 'FIFO Half: %s | ' % bool(self.fifo_half_flag)
            string += 'FIFO Full: %s | ' % bool(self.fifo_full_flag)
        elif (ptype == Packet_v1.CONFIG_READ_PACKET or ptype ==
                Packet_v1.CONFIG_WRITE_PACKET):
            if ptype == Packet_v1.CONFIG_READ_PACKET:
                string += 'Config read | '
            else:
                string += 'Config write | '
            string += 'Register: %d | ' % self.register_address
            string += 'Value: % d | ' % self.register_data
        first_splitter = string.find('|')
        string = (string[:first_splitter] + '| Chip: %d ' % self.chipid +
                string[first_splitter:])
        string += ('Parity: %d (valid: %s) ]' %
                (self.parity_bit_value, self.has_valid_parity()))
        return string

    def __repr__(self):
        return 'Packet_v1(' + str(self.bytes()) + ')'

    def bytes(self):
        padded_output = self._bit_padding + self.bits
        bytes_output = padded_output.tobytes()
        return bytes_output[::-1]

    def export(self):
        type_map = {
                self.TEST_PACKET.to01(): 'test',
                self.DATA_PACKET.to01(): 'data',
                self.CONFIG_WRITE_PACKET.to01(): 'config write',
                self.CONFIG_READ_PACKET.to01(): 'config read'
                }
        d = {}
        d['asic_version'] = self.asic_version
        d['chip_key'] = str(self.chip_key) if self.chip_key else None
        d['bits'] = self.bits.to01()
        d['type_str'] = type_map[self.packet_type.to01()]
        d['type'] = touint(self.packet_type)
        d['chipid'] = self.chipid
        d['parity'] = self.parity_bit_value
        d['valid_parity'] = self.has_valid_parity()
        ptype = self.packet_type
        if ptype == Packet_v1.TEST_PACKET:
            d['counter'] = self.test_counter
        elif ptype == Packet_v1.DATA_PACKET:
            d['channel'] = self.channel_id
            d['timestamp'] = self.timestamp
            d['adc_counts'] = self.dataword
            d['fifo_half'] = bool(self.fifo_half_flag)
            d['fifo_full'] = bool(self.fifo_full_flag)
        elif (ptype == Packet_v1.CONFIG_READ_PACKET or ptype ==
                Packet_v1.CONFIG_WRITE_PACKET):
            d['register'] = self.register_address
            d['value'] = self.register_data
        return d

    def from_dict(self, d):
        if not d['asic_version'] == self.asic_version:
            raise ValueError('invalid asic version {}'.format(d['asic_version']))
        if 'type' in d and d['type'] not in [touint(packet_type) for packet_type in \
            (Packet_v1.DATA_PACKET, Packet_v1.TEST_PACKET, Packet_v1.CONFIG_WRITE_PACKET, Packet_v1.CONFIG_READ_PACKET)]:
            raise ValueError('invalid packet type for Packet_v2')
        for key, value in d.items():
            if key in ('type_str', 'valid_parity'):
                continue
            elif key == 'bits':
                self.bits = bitarray(value)
            elif key == 'type':
                self.packet_type = value
            elif key == 'register':
                self.register_address = value
            elif key == 'value':
                self.register_data = value
            elif key == 'adc_counts':
                self.dataword = value
            elif key == 'parity':
                self.parity_bit_value = value
            elif key == 'counter':
                self.test_counter = value
            elif key == 'channel':
                self.channel_id = value
            elif key == 'fifo_half':
                self.fifo_half_flag = value
            elif key == 'fifo_full':
                self.fifo_full_flag = value
            else:
                setattr(self, key, value)

    @property
    def chip_key(self):
        if hasattr(self, '_chip_key'):
            return self._chip_key
        if self.io_group is None or self.io_channel is None:
            return None
        self._chip_key = Key(self.io_group, self.io_channel, self.chipid)
        return self._chip_key

    @chip_key.setter
    def chip_key(self, value):
        if hasattr(self, '_chip_key'):
            del self._chip_key
        if value is None:
            self.io_channel = None
            self.io_group = None
            return
        if isinstance(value, Key):
            self.io_group = value.io_group
            self.io_channel = value.io_channel
            self.chipid = value.chip_id
            return
        self.chip_key = Key(value)

    @property
    def io_group(self):
        if hasattr(self, '_io_channel'):
            return self._io_group
        return None

    @io_group.setter
    def io_group(self, value):
        if hasattr(self, '_chip_key'):
            del self._chip_key
        if value is None:
            if hasattr(self, '_io_group'):
                del self._io_group
            return
        self._io_group = value

    @property
    def io_channel(self):
        if hasattr(self, '_io_channel'):
            return self._io_channel
        return None

    @io_channel.setter
    def io_channel(self, value):
        if hasattr(self, '_chip_key'):
            del self._chip_key
        if value is None:
            if hasattr(self, '_io_channel'):
                del self._io_channel
            return
        self._io_channel = value

    @property
    def packet_type(self):
        return self.bits[self.packet_type_bits]

    @packet_type.setter
    def packet_type(self, value):
        self.bits[self.packet_type_bits] = fromuint(value,
                self.packet_type_bits)

    @property
    def chipid(self):
        return touint(self.bits[self.chipid_bits])

    @chipid.setter
    def chipid(self, value):
        if hasattr(self, '_chip_key'):
            del self._chip_key
        self.bits[self.chipid_bits] = fromuint(value,
                self.chipid_bits)

    @property
    def chip_id(self):
        return self.chipid

    @chip_id.setter
    def chip_id(self, value):
        self.chipid = value

    @property
    def parity_bit_value(self):
        return int(self.bits[self.parity_bit])

    @parity_bit_value.setter
    def parity_bit_value(self, value):
        self.bits[self.parity_bit] = bool(value)

    def compute_parity(self):
        return 1 - (self.bits[self.parity_calc_bits].count(True) % 2)

    def assign_parity(self):
        self.parity_bit_value = self.compute_parity()

    def has_valid_parity(self):
        return self.parity_bit_value == self.compute_parity()

    @property
    def channel_id(self):
        return touint(self.bits[self.channel_id_bits])

    @channel_id.setter
    def channel_id(self, value):
        self.bits[self.channel_id_bits] = fromuint(value,
                self.channel_id_bits)

    @property
    def timestamp(self):
        return touint(self.bits[self.timestamp_bits])

    @timestamp.setter
    def timestamp(self, value):
        self.bits[self.timestamp_bits] = fromuint(value,
                self.timestamp_bits)

    @property
    def dataword(self):
        ostensible_value = touint(self.bits[self.dataword_bits])
        return ostensible_value - (ostensible_value % 2)

    @dataword.setter
    def dataword(self, value):
        self.bits[self.dataword_bits] = fromuint(value,
                self.dataword_bits)

    @property
    def fifo_half_flag(self):
        return int(self.bits[self.fifo_half_bit])

    @fifo_half_flag.setter
    def fifo_half_flag(self, value):
        self.bits[self.fifo_half_bit] = bool(value)

    @property
    def fifo_full_flag(self):
        return int(self.bits[self.fifo_full_bit])

    @fifo_full_flag.setter
    def fifo_full_flag(self, value):
        self.bits[self.fifo_full_bit] = bool(value)

    @property
    def register_address(self):
        return touint(self.bits[self.register_address_bits])

    @register_address.setter
    def register_address(self, value):
        self.bits[self.register_address_bits] = fromuint(value,
                self.register_address_bits)

    @property
    def register_data(self):
        return touint(self.bits[self.register_data_bits])

    @register_data.setter
    def register_data(self, value):
        self.bits[self.register_data_bits] = fromuint(value,
                self.register_data_bits)

    @property
    def test_counter(self):
        return touint(self.bits[self.test_counter_bits_15_12] +
                self.bits[self.test_counter_bits_11_0])

    @test_counter.setter
    def test_counter(self, value):
        allbits = fromuint(value, 16)
        self.bits[self.test_counter_bits_15_12] = (
            fromuint(allbits[:4], self.test_counter_bits_15_12))
        self.bits[self.test_counter_bits_11_0] = (
            fromuint(allbits[4:], self.test_counter_bits_11_0))


# Message packet
class MessagePacket(object):
    size=72
    chip_key = None
    def __init__(self, message, timestamp):
        self.packet_type = 5
        self.message = message
        self.timestamp = timestamp

    def __str__(self):
        return '[ Message: %s | Timestamp: %d ]' % (self.message,
                self.timestamp)

    def __repr__(self):
        return 'MessagePacket(%s, %d)' % (repr(self.message),
                self.timestamp)

    def __eq__(self, other):
        return (self.message == other.message
                and self.timestamp == other.timestamp)

    def __ne__(self, other):
        return not (self == other)

    def export(self):
        return {
                'type_str': 'message',
                'type': self.packet_type,
                'message': self.message,
                'timestamp': self.timestamp,
                'bits': self.bits.to01(),
                }

    def from_dict(self, d):
        if 'type' in d and d['type'] != self.packet_type:
            raise ValueError('invalid packet type for MessagePacket')
        for key, value in d.items():
            if key == 'type':
                self.packet_type = value
            elif key == 'type_str':
                continue
            elif key == 'bits':
                self.bits = bitarray(value)
            else:
                setattr(self, key, value)

    @property
    def bits(self):
        b = bitarray()
        b.frombytes(self.bytes())
        return b

    @bits.setter
    def bits(self, value):
        value_bytes = value.tobytes()
        message_bytes = value_bytes[:64]
        timestamp_bytes = value_bytes[64:]
        self.message = message_bytes[:message_bytes.find(b'\x00')].decode()
        self.timestamp = struct.unpack('Q', timestamp_bytes)[0]

    def bytes(self):
        return (self.message.ljust(64, '\x00').encode()
                + struct.pack('Q', self.timestamp))


#-----------------------------------------------------------------------------
# Logger section
#-----------------------------------------------------------------------------

import warnings
warnings.simplefilter('default', DeprecationWarning)

class Logger(object):
    WRITE = 0
    READ = 1

    def __init__(self, enabled=False, *args, **kwargs):
        self._enabled = enabled
        self._open = False

    def record_configs(self, chips):
        pass

    def record(self, data, direction=0, *args, **kwargs):
        pass

    def is_enabled(self):
        return self._enabled

    def enable(self):
        self._enabled = True

    def disable(self):
        if self._enabled:
            self.flush()
        self._enabled = False

    def flush(self):
        pass


#-----------------------------------------------------------------------------
# HDF5 format section
#-----------------------------------------------------------------------------

num_registers = 239 #from Configuration_Lightpix_v1
_max_config_registers = num_registers

latest_version = '2.4'

dtypes = dict()
dtypes['0.0'] = {
            'raw_packet': [
                ('chip_key','S32'),
                ('type','u1'),
                ('chipid','u1'),
                ('parity','u1'),
                ('valid_parity','u1'),
                ('counter','u4'),
                ('channel','u1'),
                ('timestamp','u8'),
                ('adc_counts','u1'),
                ('fifo_half','u1'),
                ('fifo_full','u1'),
                ('register','u1'),
                ('value','u1'),
                ]
            }
dtypes['1.0'] = { # compatible with v1 packets only
            'packets': [
                ('chip_key','S32'),
                ('type','u1'),
                ('chipid','u1'),
                ('parity','u1'),
                ('valid_parity','u1'),
                ('channel','u1'),
                ('timestamp','u8'),
                ('adc_counts','u1'),
                ('fifo_half','u1'),
                ('fifo_full','u1'),
                ('register','u1'),
                ('value','u1'),
                ('counter','u4'),
                ('direction', 'u1'),
                ],
            'messages': [
                ('message', 'S64'),
                ('timestamp', 'u8'),
                ('index', 'u4'),
                ]
            }
dtypes['2.0'] = { # compatible with v2 packets and timestamp packets only
            'packets': [
                ('io_group','u1'),
                ('io_channel','u1'),
                ('chip_id','u1'),
                ('packet_type','u1'),
                ('downstream_marker','u1'),
                ('parity','u1'),
                ('valid_parity','u1'),
                ('channel_id','u1'),
                ('timestamp','u8'),
                ('dataword','u1'),
                ('trigger_type','u1'),
                ('local_fifo','u1'),
                ('shared_fifo','u1'),
                ('register_address','u1'),
                ('register_data','u1'),
                ('direction', 'u1'),
                ('local_fifo_events','u1'),
                ('shared_fifo_events','u2'),
                ('counter','u4'),
                ('fifo_diagnostics_enabled','u1'),
                ],
            'messages': [
                ('message', 'S64'),
                ('timestamp', 'u8'),
                ('index', 'u4'),
                ]
            }
dtypes['2.1'] = dtypes['2.0'].copy() # compatible with v2 packets and timestamp packets only
dtypes['2.1']['packets'].append(('first_packet','u1'))
dtypes['2.2'] = dtypes['2.1'].copy() # compatible with v2 packets, timestamp packets, sync packets, and trigger packets only
dtypes['2.3'] = dtypes['2.2'].copy() # compatible with v2 packets, timestamp packets, sync packets, and trigger packets only
dtypes['2.3']['packets'].append(('receipt_timestamp','u4'))
dtypes['2.4'] = dtypes['2.3'].copy() # compatible with v2 packets, timestamp packets, sync packets, and trigger packets only
dtypes['2.4']['configs'] = [
    ('timestamp','u8'),
    ('io_group','u1'),
    ('io_channel','u1'),
    ('chip_id','u1'),
    ('registers','({},)u1'.format(_max_config_registers))
]

dtype_property_index_lookup = dict()
dtype_property_index_lookup['0.0'] = {
            'raw_packet': {
                'chip_key': 0,
                'type': 1,
                'chipid': 2,
                'parity': 3,
                'valid_parity': 4,
                'counter': 5,
                'channel': 6,
                'timestamp': 7,
                'adc_counts': 8,
                'fifo_half': 9,
                'fifo_full': 10,
                'register': 11,
                'value': 12,
                }
            }
dtype_property_index_lookup['1.0'] = {
            'packets': {
                'chip_key': 0,
                'type': 1,
                'chipid': 2,
                'parity': 3,
                'valid_parity': 4,
                'channel': 5,
                'timestamp': 6,
                'adc_counts': 7,
                'fifo_half': 8,
                'fifo_full': 9,
                'register': 10,
                'value': 11,
                'counter': 12,
                'direction': 13,
                },
            'messages': {
                'message': 0,
                'timestamp': 1,
                'index': 2,
                }
            }
dtype_property_index_lookup['2.0'] = {
            'packets': {
                'io_group': 0,
                'io_channel': 1,
                'chip_id': 2,
                'packet_type': 3,
                'downstream_marker': 4,
                'parity': 5,
                'valid_parity': 6,
                'channel_id': 7,
                'timestamp': 8,
                'dataword': 9,
                'trigger_type': 10,
                'local_fifo': 11,
                'shared_fifo': 12,
                'register_address': 13,
                'register_data': 14,
                'direction': 15,
                'local_fifo_events': 16,
                'shared_fifo_events': 17,
                'counter': 18,
                'fifo_diagnostics_enabled': 19,
                },
            'messages': {
                'message': 0,
                'timestamp': 1,
                'index': 2,
                }
            }
dtype_property_index_lookup['2.1'] = dtype_property_index_lookup['2.0'].copy()
dtype_property_index_lookup['2.1']['packets']['first_packet'] = 20
dtype_property_index_lookup['2.2'] = dtype_property_index_lookup['2.1'].copy()
dtype_property_index_lookup['2.3'] = dtype_property_index_lookup['2.2'].copy()
dtype_property_index_lookup['2.3']['packets']['receipt_timestamp'] = 21
dtype_property_index_lookup['2.4'] = dtype_property_index_lookup['2.3'].copy()
dtype_property_index_lookup['2.4']['configs'] = {
    'timestamp': 0,
    'io_group': 1,
    'io_channel': 2,
    'chip_id': 3,
    'registers': 4
}

def _format_raw_packet_v0_0(pkt, version='0.0', dset='raw_packet', *args, **kwargs):
    dict_rep = pkt.export()
    encoded_packet = [
        dict_rep.get(key, b'') if val_type[0] == 'S'  # string
        else dict_rep.get(key, 0) for key, val_type in
        dtypes[version][dset]]
    return encoded_packet

def _parse_raw_packet_v0_0(row, message_dset, *args, **kwargs):
    if row['type'] == 4:
        return TimestampPacket(row['timestamp'])
    if row['type'] < 4:
        p = Packet_v1()
        p.chip_key = row['chip_key']
        p.packet_type = row['type']
        p.chipid = row['chipid']
        p.parity_bit_value = row['parity']
        if p.packet_type == Packet_v1.DATA_PACKET:
            p.channel = row['channel']
            p.timestamp = row['timestamp']
            p.dataword = row['adc_counts']
            p.fifo_half_flag = row['fifo_half']
            p.fifo_full_flag = row['fifo_full']
        elif p.packet_type == Packet_v1.TEST_PACKET:
            p.counter = row['counter']
        elif (p.packet_type == Packet_v1.CONFIG_WRITE_PACKET
              or p.packet_type == Packet_v1.CONFIG_READ_PACKET):
            p.register_address = row['register']
            p.register_data = row['value']
        return p
    return None


def _format_packets_packet_v1_0(pkt, version='1.0', dset='packets', *args, **kwargs):
    encoded_packet = _format_raw_packet_v0_0(pkt, *args, version=version, dset=dset, **kwargs)
    if hasattr(pkt, 'direction'):
        encoded_packet[dtype_property_index_lookup[version][dset]['direction']] = {
            Logger.WRITE: 0,
            Logger.READ: 1}[pkt.direction]
    return encoded_packet

def _format_messages_message_packet_v1_0(pkt, counter=0, *args, **kwargs):
    return (pkt.message, pkt.timestamp, counter)

def _parse_packets_v1_0(row, message_dset, *args, **kwargs):
    if row['type'] == 4:
        return TimestampPacket(row['timestamp'])
    if row['type'] == 5:
        index = row['counter']
        message_row = message_dset[index]
        message = message_row['message'].decode()
        timestamp = row['timestamp']
        return MessagePacket(message, timestamp)
    if row['type'] < 4:
        p = Packet_v1()
        p.chip_key = row['chip_key']
        p.packet_type = row['type']
        p.chipid = row['chipid']
        p.parity_bit_value = row['parity']
        if p.packet_type == Packet_v1.DATA_PACKET:
            p.channel = row['channel']
            p.timestamp = row['timestamp']
            p.dataword = row['adc_counts']
            p.fifo_half_flag = row['fifo_half']
            p.fifo_full_flag = row['fifo_full']
        elif p.packet_type == Packet_v1.TEST_PACKET:
            p.counter = row['counter']
        elif (p.packet_type == Packet_v1.CONFIG_WRITE_PACKET
              or p.packet_type == Packet_v1.CONFIG_READ_PACKET):
            p.register_address = row['register']
            p.register_data = row['value']
        p.direction = row['direction']
        return p
    return None


def _format_packets_packet_v2_0(pkt, version='2.0', dset='packets', *args, **kwargs):
    encoded_packet = _format_packets_packet_v1_0(pkt, version=version, dset=dset)
    if encoded_packet is not None:
        encoded_packet[dtype_property_index_lookup[version][dset]['packet_type']] = pkt.packet_type
        if isinstance(pkt, Packet_v2) and pkt.fifo_diagnostics_enabled:
            encoded_packet[dtype_property_index_lookup[version][dset]['fifo_diagnostics_enabled']] = 1
    return encoded_packet

def _parse_packets_v2_0(row, message_dset, *args, **kwargs):
    if row['packet_type'] == 4:
        return TimestampPacket(row['timestamp'])
    if row['packet_type'] == 5:
        index = row['counter']
        message_row = message_dset[index]
        message = message_row['message'].decode()
        timestamp = row['timestamp']
        return MessagePacket(message, timestamp)
    if row['packet_type'] < 4:
        p = Packet_v2()
        p.io_group = row['io_group']
        p.io_channel = row['io_channel']
        p.chip_id = row['chip_id']
        p.packet_type = row['packet_type']
        p.downstream_marker = row['downstream_marker']
        p.parity = row['parity']
        p.valid_parity = row['valid_parity']
        p.direction = row['direction']
        if p.packet_type == Packet_v2.DATA_PACKET:
            p.channel_id = row['channel_id']
            p.timestamp = row['timestamp']
            p.dataword = row['dataword']
            p.trigger_type = row['trigger_type']
            p.local_fifo = row['local_fifo']
            p.shared_fifo = row['shared_fifo']
            if row['fifo_diagnostics_enabled'] != 0:
                p.fifo_diagnostics_enabled = True
                p.local_fifo = row['local_fifo_events']
                p.shared_fifo = row['shared_fifo_events']
                p.timestamp = row['timestamp']
        elif p.packet_type in (Packet_v2.CONFIG_READ_PACKET, Packet_v2.CONFIG_WRITE_PACKET):
            p.register_address = row['register_address']
            p.register_data = row['register_data']
        return p
    return None


def _format_packets_packet_v2_1(pkt, version='2.1', dset='packets', *args, **kwargs):
    return _format_packets_packet_v2_0(pkt, *args, version=version, dset=dset, **kwargs)

def _parse_packets_v2_1(row, message_dset, *args, **kwargs):
    p = _parse_packets_v2_0(row, message_dset, *args, **kwargs)
    if isinstance(p, Packet_v2):
        p.first_packet = row['first_packet']
    return p

_uint8_struct = struct.Struct("<B")
def _format_packets_packet_v2_2(pkt, version='2.2', dset='packets', *args, **kwargs):
    encoded_packet = _format_packets_packet_v2_0(pkt, *args, version=version, dset=dset, **kwargs)
    if isinstance(pkt, SyncPacket):
        encoded_packet[dtype_property_index_lookup[version]['packets']['trigger_type']] = _uint8_struct.unpack(pkt.sync_type)[0]
        encoded_packet[dtype_property_index_lookup[version]['packets']['dataword']] = pkt.clk_source
    elif isinstance(pkt, TriggerPacket):
        encoded_packet[dtype_property_index_lookup[version]['packets']['trigger_type']] = _uint8_struct.unpack(pkt.trigger_type)[0]
    return encoded_packet

def _parse_packets_v2_2(row, message_dset, *args, **kwargs):
    p = _parse_packets_v2_1(row, message_dset, *args, **kwargs)
    if p is None:
        if row['packet_type'] == 6:
            return SyncPacket(
                io_group = row['io_group'],
                sync_type = _uint8_struct.pack(row['trigger_type']),
                clk_source = row['dataword'],
                timestamp = row['timestamp']
            )
        if row['packet_type'] == 7:
            return TriggerPacket(
                io_group = row['io_group'],
                trigger_type = _uint8_struct.pack(row['trigger_type']),
                timestamp = row['timestamp']
            )
    return p

def _format_packets_packet_v2_3(pkt, version='2.3', dset='packets', *args, **kwargs):
    encoded_packet = [0]*len(dtypes[version][dset])
    i = 0
    for value_name, value_type in dtypes[version][dset]:
        encoded_packet[i] = getattr(pkt, value_name, None)
        if encoded_packet[i] is None:
            if value_name == 'valid_parity' and hasattr(pkt, 'has_valid_parity'):
                encoded_packet[i] = pkt.has_valid_parity()
            elif value_type[0] == 'S': # string default
                encoded_packet[i] = ''
            else:
                encoded_packet[i] = 0
        i += 1
    if pkt.packet_type == 6: # sync packets
        encoded_packet[dtype_property_index_lookup[version]['packets']['trigger_type']] = _uint8_struct.unpack(pkt.sync_type)[0]
        encoded_packet[dtype_property_index_lookup[version]['packets']['dataword']] = pkt.clk_source
    elif pkt.packet_type == 7: # trigger packets
        encoded_packet[dtype_property_index_lookup[version]['packets']['trigger_type']] = _uint8_struct.unpack(pkt.trigger_type)[0]
    return encoded_packet

def _parse_packets_v2_3(row, message_dset, *args, **kwargs):
    p = _parse_packets_v2_2(row, message_dset, *args, **kwargs)
    if isinstance(p, Packet_v2):
        p.receipt_timestamp = row['receipt_timestamp']
    return p

def _format_configs_chip_v2_4(chip, version='2.4', dset='configs', timestamp=0, *args, **kwargs):
    row = np.zeros((1,),dtype=dtypes[version][dset])
    row['timestamp'] = timestamp
    row['io_group'] = chip.io_group
    row['io_channel'] = chip.io_channel
    row['chip_id'] = chip.chip_id
    endian='big' if chip.asic_version == 1 else 'little'
    for i,bits in enumerate(chip.config.all_data()):
        row['registers'][0,i] = touint(bits, endian=endian)
    return row

_parse_method_lookup = {
    '0.0': {
        'raw_packet': _parse_raw_packet_v0_0
    },
    '1.0': {
        'packets': _parse_packets_v1_0
    },
    '2.0': {
        'packets': _parse_packets_v2_0
    },
    '2.1': {
        'packets': _parse_packets_v2_1
    },
    '2.2': {
        'packets': _parse_packets_v2_2
    },
    '2.3': {
        'packets': _parse_packets_v2_3
    },
    '2.4': {
        'packets': _parse_packets_v2_3
    }
}

def from_file(filename, version=None, start=None, end=None, load_configs=None):
    with h5py.File(filename, 'r') as f:
        file_version = f['_header'].attrs['version']
        if version is None:
            version = file_version
        elif version[0] == '~':
            file_major, _, file_minor = file_version.split('.')
            version_major, _, version_minor = version.split('.')
            version_major = version_major[1:]
            if (file_major != version_major
                    or file_minor < version_minor):
                raise RuntimeError('Incompatible versions: existing: %s, '
                    'specified: %s' % (file_version, version))
            else:
                version = file_version
        elif version == file_version:
            pass
        else:
            raise RuntimeError('Incompatible versions: existing: %s, '
                'specified: %s' % (file_version, version))

        if version not in dtypes:
            raise RuntimeError('Unknown version: %s' % version)

        if version == '0.0':
            dset_name = 'raw_packet'
            message_dset = None
            configs_dset = None
        else:
            dset_name = 'packets'
            message_dset_name = 'messages'
            message_props = (
                    dtype_property_index_lookup[version][message_dset_name])
            message_dset = f[message_dset_name]

        props = dtype_property_index_lookup[version][dset_name]
        packets = []
        if start is None and end is None:
            dset_iter = f[dset_name]
        else:
            dset_iter = f[dset_name][start:end]
        for row in dset_iter:
            pkt = _parse_method_lookup[version][dset_name](row, message_dset)
            if pkt is not None:
                packets.append(pkt)

        configs = []
        if version >= '2.4':
            dset_name ='configs'
            if load_configs:
                if isinstance(load_configs,bool):
                    dset_iter = f[dset_name]
                else:
                    dset_iter = f[dset_name][load_configs]
                asic_version = f[dset_name].attrs['asic_version']
                for row in dset_iter:
                    chip = _parse_method_lookup[version][dset_name](row, asic_version=asic_version)
                    if chip is not None:
                        configs.append(chip)
        return {
                'packets': packets,
                'configs': configs,
                'created': f['_header'].attrs['created'],
                'modified': f['_header'].attrs['modified'],
                'version': f['_header'].attrs['version'],
                }


#-----------------------------------------------------------------------------
# Pacman format section
#-----------------------------------------------------------------------------

import struct
from bidict import bidict
import time

#: Most up-to-date message format version.
latest_version = '0.0'

HEADER_LEN=8
WORD_LEN=16
MSG_TYPE_DATA=b'D'
MSG_TYPE_REQ=b'?'
MSG_TYPE_REP=b'!'
WORD_TYPE_DATA=b'D'
WORD_TYPE_TRIG=b'T'
WORD_TYPE_SYNC=b'S'
WORD_TYPE_PING=b'P'
WORD_TYPE_WRITE=b'W'
WORD_TYPE_READ=b'R'
WORD_TYPE_TX=WORD_TYPE_DATA
WORD_TYPE_PONG=WORD_TYPE_PING
WORD_TYPE_ERR=b'E'

msg_type_table = bidict([
    ('REQ',MSG_TYPE_REQ),
    ('REP',MSG_TYPE_REP),
    ('DATA',MSG_TYPE_DATA)
    ])
word_type_table = dict(
    REQ=bidict([
        ('PING',WORD_TYPE_PING),
        ('WRITE',WORD_TYPE_WRITE),
        ('READ',WORD_TYPE_READ),
        ('TX',WORD_TYPE_TX)
        ]),
    REP=bidict([
        ('WRITE',WORD_TYPE_WRITE),
        ('READ',WORD_TYPE_READ),
        ('TX',WORD_TYPE_TX),
        ('PONG',WORD_TYPE_PONG),
        ('ERR',WORD_TYPE_ERR)
        ]),
    DATA=bidict([
        ('DATA',WORD_TYPE_DATA),
        ('TRIG',WORD_TYPE_TRIG),
        ('SYNC',WORD_TYPE_SYNC)
        ])
    )

msg_header_fmt = '<cLxH'
msg_header_struct = struct.Struct(msg_header_fmt)

word_fmt_table = dict(
    DATA='<cBLxx8s',
    TRIG='<2cxxL8x',
    SYNC='<2cBxL8x',
    PING='<c15x',
    WRITE='<c3xL4xL',
    READ='<c3xL4xL',
    TX='<cB6x8s',
    PONG='<c15x',
    ERR='<cB14s'
    )
word_struct_table = dict([
    (word_type, struct.Struct(word_fmt))
    for word_type, word_fmt in word_fmt_table.items()
    ])

def format_header(msg_type, msg_words):
    return msg_header_struct.pack(
        msg_type_table[msg_type],
        int(time.time()),
        msg_words
        )

def format_word(msg_type, word_type, *data):
    return word_struct_table[word_type].pack(
        word_type_table[msg_type][word_type],
        *data
        )

def parse_header(msg):
    msg_header_data = msg_header_struct.unpack(msg[:HEADER_LEN])
    return (msg_type_table.inv[msg_header_data[0]],) + tuple(msg_header_data[1:])

def parse_word(msg_type, word):
    word_type = word_type_table[msg_type].inv[word[0:1]]
    return (word_type,) + tuple(word_struct_table[word_type].unpack(word)[1:])

def format_msg(msg_type, msg_words):
    bytestream = format_header(msg_type, len(msg_words))
    for msg_word in msg_words:
        bytestream += format_word(msg_type, *msg_word)
    return bytestream

def parse_msg(msg):
    header = parse_header(msg)
    words = list()
    for idx in range(HEADER_LEN,len(msg),WORD_LEN):
        words.append(parse_word(
            header[0],
            msg[idx:idx+WORD_LEN]
            ))
    return header, words

def _replace_none(obj, attr, default=0):
    return getattr(obj, attr) if getattr(obj, attr) is not None else default

def _packet_data_req(pkt, *args):
    if isinstance(pkt, Packet_v2):
        return ('TX',
                _replace_none(pkt,'io_channel'),
                pkt.bytes())
    return tuple()

def _packet_data_data(pkt, ts_pacman, *args):
    if isinstance(pkt, Packet_v2):
        return ('DATA',
                _replace_none(pkt,'io_channel'),
                pkt.receipt_timestamp if hasattr(pkt,'receipt_timestamp') else ts_pacman,
                pkt.bytes())
    elif isinstance(pkt, SyncPacket):
        return ('SYNC',
                _replace_none(pkt,'sync_type'),
                _replace_none(pkt,'clk_source'),
                _replace_none(pkt,'timestamp'))
    elif isinstance(pkt, TriggerPacket):
        return ('TRIG',
                _replace_none(pkt,'trigger_type'),
                _replace_none(pkt,'timestamp'))
    return tuple()

def format(packets, msg_type='REQ', ts_pacman=0):
    get_data = _packet_data_req
    if msg_type == 'DATA':
        get_data = _packet_data_data

    word_datas = list()
    for packet in packets:
        word_data = get_data(packet, ts_pacman)
        if len(word_data) == 0: continue
        word_datas.append(word_data)
    return format_msg(msg_type, word_datas)

def parse(msg, io_group=None):
    packets = list()
    header, word_datas = parse_msg(msg)
    packets.append(TimestampPacket(timestamp=header[1]))
    packets[0].io_group = io_group
    for word_data in word_datas:
        packet = None
        if word_data[0] in ('TX', 'DATA'):
            packet = Packet_v2(word_data[-1])
            packet.receipt_timestamp = word_data[2]
            packet.io_group = io_group
            packet.io_channel = word_data[1]
        elif word_data[0] == 'TRIG':
            packet = TriggerPacket(trigger_type=word_data[1], timestamp=word_data[2])
            packet.io_group = io_group
        elif word_data[0] == 'SYNC':
            packet = SyncPacket(sync_type=word_data[1], clk_source=word_data[2] & 0x01, timestamp=word_data[3])
            packet.io_group = io_group
        if packet is not None:
            packets.append(packet)
    return packets


#-----------------------------------------------------------------------------
# Application section
#-----------------------------------------------------------------------------
'''
#example usage
datafile = "example-pacman-data.h5"

print(1)
packets = from_file(datafile)['packets'] #read from HDF5 file
print(2)
msg_breaks = [i for i in range(len(packets)) if packets[i].packet_type == 4 or i == len(packets)-1] #find the timestamp packets which signify message breaks
print(3)
msg_packets = [packets[i:j] for i,j in zip(msg_breaks[:-1], msg_breaks[1:])] #separate into messages
print(4)
msgs = [format(p, msg_type='DATA') for p in msg_packets]
print(5)
word_lists = [parse_msg(p)[1] for p in msgs] #retrieve lists of words from each message
print(6)
for i in word_lists:
    print(format_msg('DATA',i))
'''