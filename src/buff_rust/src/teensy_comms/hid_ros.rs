use crate::teensy_comms::{data_structures::*, hid_layer::*};
use rosrust_msg::std_msgs;
use std::time::Instant;

pub static PUBLISH_TIME_MS: u128 = 100;

pub struct HidROS {
    pub layer: HidLayer,

    pub proc_state_pubs: Vec<rosrust::Publisher<std_msgs::Float64MultiArray>>,
    pub proc_output_pubs: Vec<rosrust::Publisher<std_msgs::Float64MultiArray>>,
}

impl HidROS {
    /// Handles publishing data to ROS from HID
    ///
    /// # Usage
    /// ```
    /// use buff_rust::teensy_comms::buff_hid::HidROS;
    ///
    /// let hidros = HidROS::new();
    /// ```
    pub fn new(layer: HidLayer) -> HidROS {
        let robot_status = RobotStatus::from_self();
        let proc_names = robot_status.get_process_names();

        env_logger::init();
        rosrust::init("buffpy_hid");

        let proc_state_pubs = proc_names
            .iter()
            .map(|name| rosrust::publish(format!("{}_state", name).as_str(), 1).unwrap())
            .collect();

        let proc_output_pubs = proc_names
            .iter()
            .map(|name| rosrust::publish(format!("{}_output", name).as_str(), 1).unwrap())
            .collect();

        HidROS {
            layer: layer,

            proc_state_pubs: proc_state_pubs,
            proc_output_pubs: proc_output_pubs,
        }
    }

    // /// Publish sensor data to ROS
    // pub fn publish_sensors(&self) {
    //     self.sensor_pubs
    //         .iter()
    //         .enumerate()
    //         .for_each(|(i, sensor_pub)| {
    //             let mut msg = std_msgs::Float64MultiArray::default();
    //             msg.data = self.robot_status.sensors[i].read().unwrap().data();
    //             msg.data
    //                 .push(self.robot_status.sensors[i].read().unwrap().timestamp());
    //             sensor_pub.send(msg).unwrap();
    //         });
    // }

    /// Publish sensor data to ROS
    pub fn publish_process_context(&self) {
        self.proc_state_pubs
            .iter()
            .enumerate()
            .for_each(|(i, proc_state_pub)| {
                let mut msg = std_msgs::Float64MultiArray::default();
                msg.data = self.layer.get_context(i);
                msg.data.push(self.layer.process_timestamp(i));
                proc_state_pub.send(msg).unwrap();
            });
    }

    pub fn publish_process_outputs(&self) {
        self.proc_output_pubs
            .iter()
            .enumerate()
            .for_each(|(i, proc_out_pub)| {
                let mut msg = std_msgs::Float64MultiArray::default();
                msg.data = self.layer.get_output(i);
                msg.data.push(self.layer.process_timestamp(i));
                proc_out_pub.send(msg).unwrap();
            });
    }

    /// Start to continually publish motor and sensor data
    pub fn spin(&mut self) {
        println!("HID-ROS Live");

        while rosrust::is_ok() {
            let loopt = Instant::now();

            // self.publish_sensors();

            if loopt.elapsed().as_millis() > 5 {
                println!("HID ROS over cycled {}", loopt.elapsed().as_micros());
            }
            self.layer.delay(loopt);
        }

        // buffpy RUN relies on ros to shutdown
        self.layer.shutdown();
    }

    /// Begin publishing motor, controller, and sensor data to ROS and
    /// sending control reports to [HidWriter]
    ///
    /// # Example
    /// See [HidLayer::pipeline()]
    pub fn pipeline(&mut self) {
        while !self.layer.is_connected() {}

        println!("HID-ROS Live");

        let mut publish_timer = Instant::now();
        let mut pub_switch = 0;

        while rosrust::is_ok() && !self.layer.is_shutdown() {
            let loopt = Instant::now();

            // don't publish every cycle
            if publish_timer.elapsed().as_millis() > PUBLISH_TIME_MS {
                publish_timer = Instant::now();
                match pub_switch {
                    0 => {
                        self.publish_process_context();
                        pub_switch += 1;
                    }
                    1 => {
                        self.publish_process_outputs();
                        pub_switch = 0;
                    }
                    _ => {}
                }
            }

            self.layer.delay(loopt);
        }

        // buffpy RUN relies on ros to shutdown
        self.layer.shutdown();
    }
}
