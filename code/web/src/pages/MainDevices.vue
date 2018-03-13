<!--
This vue.js file is the main display for existing devices.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="Devices" subtitle="Configure devices that are discoverable by Alexa and bundle several IR commands."
  :sublinks="[{linkto:'/irmonitor', linktext:'IR Monitor'},
              {linkto:'/devices', linktext:'Devices'},
              {linkto:'/bookmarks', linktext:'Bookmarks'}]"></TopNav>
  <section class="section">
    <table class="table is-narrow is-fullwidth is-hoverable">
      <col width="33%">
      <col width="33%">
      <col width="33%">
      <col>
      <thead>
        <tr>
          <th><abbr>Name</abbr></th>
          <th><abbr>on</abbr></th>
          <th><abbr>off</abbr></th>
          <th></th>
        </tr>
      </thead>
      <tbody>
        <tr v-if="Object.keys(devices).length==0">
          <td colspan="4">
            <div class="level">
              <div class="level-item has-text-centered">
                <div>
                  <p class="title">No devices found</p>
                </div>
              </div>
            </div>
          </td>
        </tr>
        <tr style="cursor: pointer;" v-for="key in Object.keys(devices)" v-else>
          <td>
            {{key}}
          </td>
          <td>
            <template v-if="'on' in devices[key]">
              <div class="field is-grouped is-grouped-multiline">
                <template v-for="action in devices[key].on">
                  <template v-if="'bookmark' in action">
                    <div class="control">
                      <div class="tags has-addons">
                        <span class="tag">a</span>
                        <span class="tag is-info">{{action.bookmark}}</span>
                      </div>
                    </div>
                  </template>
                  <template v-else-if="'pause' in action">
                    <div class="control">
                      <div class="tags has-addons">
                        <span class="tag">p</span>
                        <span class="tag is-warning">{{action.pause}}</span>
                      </div>
                    </div>
                  </template>
                </template>
              </div>
            </template>
            <template v-else>
              <span class="tag is-danger">none</span>
            </template>
          </td>
          <td>
            <template v-if="'off' in devices[key]">
              <div class="field is-grouped is-grouped-multiline">
                <template v-for="action in devices[key].off">
                  <template v-if="'bookmark' in action">
                    <div class="control">
                      <div class="tags has-addons">
                        <span class="tag">a</span>
                        <span class="tag is-info">{{action.bookmark}}</span>
                      </div>
                    </div>
                  </template>
                  <template v-else-if="'pause' in action">
                    <div class="control">
                      <div class="tags has-addons">
                        <span class="tag">p</span>
                        <span class="tag is-warning">{{action.pause}}</span>
                      </div>
                    </div>
                  </template>
                </template>
              </div>
            </template>
            <template v-else>
              <span class="tag is-danger">none</span>
            </template>
          </td>
          <td>
            <div class="field has-addons is-right">
              <a class="control button is-small" v-on:click="sendIR(key,'on')" :disabled="!('on' in devices[key])">
                <span class="icon">
                  <i class="icon-send"></i>
                </span>
                <span><span class="is-hidden-touch">Send </span>on</span>
              </a>

              <a class="control button is-small" v-on:click="sendIR(key,'off')" :disabled="!('off' in devices[key])">
                <span class="icon">
                  <i class="icon-send"></i>
                </span>
                <span><span class="is-hidden-touch">Send </span>off</span>
              </a>

              <a class="control button is-small is-hidden-touch" v-on:click="startEditDevice(key)">
                <span class="icon">
                  <i class="icon-gear"></i>
                </span>
                <span>Edit</span>
              </a>
              <a class="control button is-small is-hidden-desktop" v-on:click="startEditDevice(key)">
                <span class="icon">
                  <i class="icon-gear"></i>
                </span>
              </a>

              <a class="control button is-small is-danger is-hidden-touch" v-on:click="deleteDevice(key)">
                <span class="icon">
                  <i class="icon-minus"></i>
                </span>
                <span>Delete</span>
              </a>
              <a class="control button is-small is-danger is-hidden-desktop" v-on:click="deleteDevice(key)">
                <span class="icon">
                  <i class="icon-minus"></i>
                </span>
              </a>
            </div>
          </td>
        </tr>
      </tbody>
    </table>
    <div class="level">
      <div class="level-item">
        <div class="buttons">
          <a class="button is-primary" v-on:click="startAddDevice()">
            <span class="icon">
              <i class="icon-plus"></i>
            </span>
            <span>Add new</span>
          </a>
          <a class="button is-primary" v-on:click="rebootESP()">
            <span class="icon">
              <i class="icon-repeat"></i>
            </span>
            <span>Refresh Alexa Devices</span>
          </a>
        </div>
      </div>
    </div>
    <div class="modal is-active" v-if="editModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalDevice :bookmarks="bookmarks" :name="activeDevice.name" :actions="activeDevice.actions" dialogTitle="Edit Device" v-on:cancel="editModalShown=false" v-on:commit="editDevice"></ModalDevice>
      </div>
    </div>
    <div class="modal is-active" v-else-if="addModalShown">
      <div class="modal-background"></div>
      <div class="modal-content">
        <ModalDevice :bookmarks="bookmarks" dialogTitle="Add Device" v-on:cancel="addModalShown=false" v-on:commit="addDevice"></ModalDevice>
      </div>
    </div>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import ModalDevice from '../components/ModalDevice.vue'

import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'MainDevices',
  components: {
    ModalDevice
  },
  computed: {
  },
  data() {
    return {
      editModalShown: false,
      addModalShown: false,
      activeDevice: undefined,
      devices: {},
      bookmarks: [],
      // the protocols are a copy from the protocols defined in IRRemoteESP8266, enum decode_type_t in IRRemoteESP8266.h
      protocols: ["UNUSED",
        "RC5",
        "RC6",
        "NEC",
        "SONY",
        "PANASONIC",
        "JVC",
        "SAMSUNG",
        "WHYNTER",
        "AIWA_RC_T501",
        "LG",
        "SANYO",
        "MITSUBISHI",
        "DISH",
        "SHARP",
        "COOLIX",
        "DAIKIN",
        "DENON",
        "KELVINATOR",
        "SHERWOOD",
        "MITSUBISHI_AC",
        "RCMM",
        "SANYO_LC7461",
        "RC5X",
        "GREE",
        "PRONTO", // Technically not a protocol, but an encoding.
        "NEC_LIKE",
        "ARGO",
        "TROTEC",
        "NIKAI",
        "RAW", // Technically not a protocol, but an encoding.
        "GLOBALCACHE", // Technically not a protocol, but an encoding.
        "TOSHIBA_AC",
        "FUJITSU_AC",
        "MIDEA",
        "MAGIQUEST",
        "LASERTAG",
        "CARRIER_AC"
      ]
    }
  },
  methods: {
    // calls the api function to delete a device
    deleteDevice: function(devicename) {
      var loadingComponent=this.$loading.open();

      HTTP.delete('/api/devices/'+devicename,{timeout:5000}).then((response) => {
        this.$toast.open({
          message: 'Device \"'+devicename+'\" has been deleted.',
          type: 'is-success',
          position: 'is-bottom'
        });

        delete this.devices[devicename];
        this.$forceUpdate();
      }).catch((e) => {
        if(e.response && e.response.status==404) {
          this.$toast.open({
            message: 'Device \"'+devicename+'\" has not been found.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else {
          this.$toast.open({
            message: 'Unknown error deleting device \"'+devicename+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      }).then(() => {
        loadingComponent.close();
      })
    },
    // sends the IR code for a specific device and a specific state
    // state==true means on, state==false means off
    sendIR: function(devicename,state) {
      HTTP.get('api/sendir?device='+devicename+'&status='+state,{timeout: 5000}).then((response) => {
        this.$toast.open({
          message: 'IR code for \"'+devicename+', '+state+'\" has been sent.',
          type: 'is-success',
          position: 'is-bottom'
        });
      }).catch((e) => {
        if(e.response && e.response.status==404) {
          this.$toast.open({
            message: 'Device \"'+devicename+'\" not found.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else if(e.response && e.response.status==400) {
          this.$toast.open({
            message: 'Request didn\'t contain all required information.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else {
          this.$toast.open({
            message: 'Unknown error sending device \"'+devicename+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      })
    },
    // displays the edit modal
    startEditDevice: function(devicename) {
      this.activeDevice={"name": devicename,
                         "actions": {"on": ("on" in this.devices[devicename] ? this.devices[devicename].on : []),
                                     "off": ("off" in this.devices[devicename] ? this.devices[devicename].off : [])}};
      this.editModalShown=true;
    },
    // callback for the edit modal once the edit has been finished and the user commits the edits
    editDevice: function(device,loadingComponent) {
      HTTP.put('api/devices/'+this.activeDevice.name,device,{timeout: 5000}).then((response) => {
        this.$toast.open({
          message: 'Device \"'+this.activeDevice.name+'\" has been modified.',
          type: 'is-success',
          position: 'is-bottom'
        });

        this.activeDevice=undefined;
        this.editModalShown=false;
      }).catch((e) => {
        if(e.response && e.response.status==409) {
          this.$toast.open({
            message: 'Device \"'+device.name+'\" already exists.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else if(e.response && e.response.status==400) {
          this.$toast.open({
            message: 'Request didn\'t contain all required information.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else {
          this.$toast.open({
            message: 'Unknown error editing device \"'+this.activeDevice.name+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      }).then(() => {
        loadingComponent.close();
      })
    },
    // shows the add device modal
    startAddDevice: function() {
      this.activeBookmark=undefined;
      this.addModalShown=true;
    },
    // callback function for the add device modal once the user commits
    addDevice: function(device,loadingComponent) {
      HTTP.post('api/devices',device,{timeout: 5000}).then((response) => {
        this.$toast.open({
          message: 'Device \"'+device.name+'\" has been added.',
          type: 'is-success',
          position: 'is-bottom'
        });

        this.devices[device.name]={"on": device.actions.on, "off": device.actions.off};

        this.activeDevice=undefined;
        this.addModalShown=false;
      }).catch((e) => {
        if(e.response && e.response.status==409) {
          this.$toast.open({
            message: 'Device \"'+device.name+'\" already exists.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else if(e.response && e.response.status==400) {
          this.$toast.open({
            message: 'Request didn\'t contain all required information.',
            type: 'is-danger',
            position: 'is-bottom'
          });
        } else {
          this.$toast.open({
            message: 'Unknown error creating device \"'+device.name+'\".',
            type: 'is-danger',
            position: 'is-bottom'
          });
        }
      }).then(() => {
        loadingComponent.close();
      })
    },
    rebootESP: function() {
      HTTP.get('/api/reboot').catch((e) => {});
    }
  },
  // on load, get the available devices from the ESP
  created: function() {
    HTTP.get('/api/devices').then((response) => {
      if (response.status == 200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json"))) {
        this.devices=response.data;
      } else {
        this.devices = {};
      }
    }).catch((e) => {
      this.devices = {};
      this.$toast.open({
        message: 'Couldn\'t get devices from ESP.',
        type: 'is-danger',
        position: 'is-bottom'
      });
    })
  }
}
</script>

<style scoped>

</style>
