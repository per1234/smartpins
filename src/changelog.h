#define SMARTPINS_VERSION "0.2.2"
/*
	Authors:
							PMB	Phil Bowles		<philbowles2012@gmail.com>
	TODO:
				fix docs fpr 0.2.2 (remove hysteresis from retrig)

	Changelog:					

		1/12/2017	0.2.2	PMB	remove hysteresis from retrig - cause more probs than solved
								change retriggering to send cooked on every (re)trigger
								[prob shud have always been thus - and allows EXTERNAL timout handling]
								
		12/10/2017	0.2.1	PMB	fix problem of retriggering already "active" on boot
								fix for retriggering HW staying active longer than t/o period
								fix absent retriggering RAW notification
								
 		1/10/2017 	0.1.0	PMB initial release

*/