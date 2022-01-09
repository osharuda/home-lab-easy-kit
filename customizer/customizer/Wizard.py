import sys
from HLEKWizard import HLEKWizard

devname = sys.argv[1]
wizard_name = sys.argv[2].lower()
remove_dev = sys.argv[3].lower() == 'remove'

wizard1 = HLEKWizard(devname)
wizard1.load(wizard_name)
wizard1.run(remove_dev)