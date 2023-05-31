# Copyright 2023 OpenHW Group
#
# Licensed under the Solderpad Hardware Licence, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://solderpad.org/licenses/
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

check_sec -setup -spec_top cve2_core -imp_top cve2_core \
        -spec_analyze  "-sv -f ./golden.src" \
        -imp_analyze "-sv -f ./revised.src"\
        -auto_map_reset_x_values


clock clk_i
reset ~rst_ni

check_sec -map -auto

check_sec -waive -waive_signals test_en_i

check_sec -prove

check_sec -signoff -get_valid_status -summary -file $report_dir/summary.cadence.log
