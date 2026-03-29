import sys
if sys.prefix == '/Users/carlosgaratemartin-ondarza/anaconda3/envs/ROS2':
    sys.real_prefix = sys.prefix
    sys.prefix = sys.exec_prefix = '/Users/carlosgaratemartin-ondarza/ros2_crystal/control_gui/install/control_gui'
