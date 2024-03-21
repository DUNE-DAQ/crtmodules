// This is the configuration schema for timinglibs

local moo = import "moo.jsonnet";
local nc = moo.oschema.numeric_constraints;

local stypes = import "daqconf/types.jsonnet";
local types = moo.oschema.hier(stypes).dunedaq.daqconf.types;

local sboot = import "daqconf/bootgen.jsonnet";
local bootgen = moo.oschema.hier(sboot).dunedaq.daqconf.bootgen;

local ns = "dunedaq.crtmodules.confgen";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local cs = {
  usb_serial_selector:   s.number('usb_serial',   dtype='i4', constraints=nc(minimum=0, maximum=50), doc='USB serial number selector: 3, 13, 14, 22'),
  pmt_board_selector:    s.number('pmt_board',    dtype='i4', constraints=nc(minimum=0, maximum=31), doc='PMT board number selector (0-31)'),
  gain_selector:         s.number('gain',         dtype='i4', constraints=nc(minimum=10, maximum=32),doc='Channel gain selector: (10-32)'),
  dac_threshold_selector:s.number('dac_threshold',dtype='i8', constraints=nc(minimum=700, maximum=2000), doc='DAC threshold setting (700-2000'),
  hv_setting_selector:   s.number('hv_setting',   dtype='i8', constraints=nc(minimum=0, maximum=900),doc='HV setting (0-900 V)'),
  pipedelay_selector:    s.number('pipedelay',    dtype='i4', constraints=nc(minimum=0, maximum=100),  doc='Pipe delay (0-100 clock cycles)'), 
  force_trigger_selector:s.number('force_trigger',dtype='i4', constraints=nc(minimum=0, maximum=11),  doc='How often to force a trigger (00, 01, 10, 11)'), 
  trigger_mode_selector: s.number('trigger_mode', dtype='i4', constraints=nc(minimum=0, maximum=100),  doc='Trigger mode'), 
  num_modules_selector:  s.number('num_crtcontrollermodules', dtype='i4', constraints=nc(minimum=0, maximum=100),  doc='Number of CRT controller modules'), 

  gainlist: s.sequence('gains', self.gain_selector, doc='Sequence of gain constants'),

  crtmodule: s.record('crtmodule', [
    s.field('gain',          self.gainlist,              default=[],          doc='List of gain constants'),
    s.field('usb_serial',    self.usb_serial_selector,   default=3,           doc='USB serial number selector (3, 13, 14, 22)'),
    s.field('pmt_board',     self.pmt_board_selector,    default=0,           doc='PMT board number selector (0-31)'),
    s.field('dac_threshold', self.dac_threshold_selector,default=1200,        doc='DAC threshold setting (700-2000)'),
    s.field('hv_setting',    self.hv_setting_selector,   default=750,         doc='HV setting (0-900 V)'),
    s.field('use_maroc2gain',types.flag,                 default=true,        doc='Switch to use the custom gain constants defined here'),
    s.field('gate',          types.string,               default="off",       doc='Type of gate to use'),
    s.field('force_trigger', self.force_trigger_selector,default=0,           doc='How often to force a trigger (00, 01, 10, 11)'),
    s.field('trigger_mode',  self.trigger_mode_selector, default=16,          doc='Trigger mode'),
    s.field('pipedelay',     self.pipedelay_selector,    default=0,           doc='Pipe delay (0-100 clock cycles)'),
  ]),

  crtmodulelist: s.sequence('crtmodulelist', self.crtmodule, doc='Sequence of CRT modules'),

  crtmodules: s.record('crtmodules', [
      s.field('crtmodule_list', self.crtmodulelist, default=[], doc='wrapper for list of modules'),
      s.field('num_crtcontrollermodules', self.num_modules_selector, default=1, doc='Number of CRT controller modules'),
    ], doc='wrapper for list of modules'),

  crtmodules_gen: s.record("crtmodules_gen", [
      s.field("boot", bootgen.boot, default=bootgen.boot, doc="Boot parameters"),
      s.field("crtmodules", self.crtmodules, default=self.crtmodules, doc="crtmodules parameters")
  ]),
};

// Output a topologically sorted array.
stypes + sboot + moo.oschema.sort_select(cs, ns)
