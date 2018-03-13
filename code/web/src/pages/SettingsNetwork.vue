<!--
This vue.js file is the main settings page for network settings.
-->
<template>
<div id="app" class="is-marginless">
  <TopNav title="Settings - Network" subtitle="Network settings of the IRBridge can be configured here"
  :sublinks="[{linkto:'/settings/network', linktext:'Network'},
              {linkto:'/settings/backup', linktext:'Backup/Restore'}]"></TopNav>
  <section class="section">
    <form v-on:submit.prevent="commitChanges()">
    <div class="container">
      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Name</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input class="input" type="text" placeholder="Flipdisplay name" v-model="SettingsNetwork.ESPName">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">AP Password</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input class="input" type="password" placeholder="Password in AP mode" v-model="SettingsNetwork.APPassword" v-on:change="SettingsNetwork.APPassword_changed=true">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Wifi name</label>
        </div>
        <div class="field-body">
          <div class="field has-addons">
            <p class="control is-expanded">
              <input class="input" type="text" placeholder="AP to connect to" v-model="SettingsNetwork.WiFiName">
            </p>
            <p class="control">
              <a class="button is-info" v-on:click="scanWifis()" v-bind:class="{'is-loading': scanWorking}" :disabled="scanWorking">
                <span class="icon">
                  <i class="icon-search"></i>
                </span>
                <span>Search</span>
              </a>
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal" v-if="NetworksInRange.length>0">
        <div class="field-label is-normal">
          <label class="label"></label>
        </div>
        <div class="field-body">
          <div class="field has-addons">
            <table class="table is-narrow is-fullwidth is-hoverable">
              <thead>
                <tr>
                  <th><abbr title="Signal strength">RSSI</abbr></th>
                  <th><abbr title="Wifi name">SSID</abbr></th>
                  <th><abbr title="Wifi encryption">Enc.</abbr></th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="network in sortedNetworksInRange" v-on:click="selectWifi(network)" style="cursor: pointer;"
>
                  <td>
                    <span class="icon">
                      <i v-bind:class="'icon-wifi-'+getIconNumber(network.rssi)"></i>
                    </span>
                  </td>
                  <th>{{network.ssid}}</th>
                  <td>
                    <span class="icon">
                      <i v-bind:class="[network.enc==7 ? 'icon-unlock-alt' : 'icon-lock']"></i>
                    </span>
                    {{WifiEncryption[network.enc]}}
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Wifi security</label>
        </div>
        <div class="field-body">
          <div class="field has-addons">
            <p class="control">
              <span class="select">
                <select  v-model="SettingsNetwork.WiFiSecurity">
                  <option value=7>{{WifiEncryption[7]}}</option>
                  <option value=5>{{WifiEncryption[5]}}</option>
                  <option value=2>{{WifiEncryption[2]}}</option>
                  <option value=4>{{WifiEncryption[4]}}</option>
                  <option value=8>{{WifiEncryption[8]}}</option>
                </select>
              </span>
            </p>
            <p class="control is-expanded">
              <input required :disabled="SettingsNetwork.WiFiSecurity=='7'" class="input" type="password" placeholder="Password" v-model="SettingsNetwork.WiFiPassword" v-on:change="SettingsNetwork.WiFiPassword_changed=true">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal is-narrow">
        <div class="field-label">
          <label class="label">DHCP</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input type="checkbox" v-model="SettingsNetwork.useDHCP"> Use DHCP?
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">IP</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input required :disabled="SettingsNetwork.useDHCP" class="input" type="text" placeholder="IP address" pattern="^([0-9]{1,3}\.){3}[0-9]{1,3}$" v-model="SettingsNetwork.WiFiIP">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Subnet</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input required :disabled="SettingsNetwork.useDHCP" class="input" type="text" placeholder="Subnet mask" pattern="^([0-9]{1,3}\.){3}[0-9]{1,3}$" v-model="SettingsNetwork.WiFiSubnet">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label is-normal">
          <label class="label">Gateway</label>
        </div>
        <div class="field-body">
          <div class="field">
            <p class="control">
              <input required :disabled="SettingsNetwork.useDHCP" class="input" type="text" placeholder="Gateway IP" pattern="^([0-9]{1,3}\.){3}[0-9]{1,3}$" v-model="SettingsNetwork.WiFiGateway">
            </p>
          </div>
        </div>
      </div>

      <div class="field is-horizontal">
        <div class="field-label">
          <!-- Left empty for spacing -->
        </div>
        <div class="field-body">
          <div class="field">
            <div class="control">
              <button class="button is-primary" v-bind:class="{'is-loading': commitWorking}" :disabled="commitWorking" type="submit">
                <span class="icon">
                  <i class="icon-save"></i>
                </span>
                <span>Save</span>
              </button>
            </div>
          </div>
        </div>
      </div>
    </div>
    </form>
  </section>
  <BottomFooter></BottomFooter>
</div>
</template>

<script>
import {HTTP} from '../plugins/axios'

export default {
  name: 'SettingsNetwork',
  components: {
  },
  data() {
    return {
      SettingsNetwork: {ESPName: "",
                        APPassword:"",
                        APPassword_changed: false,
                        WiFiName:"",
                        WiFiPassword:"",
                        WiFiPassword_changed: false,
                        WiFiSecurity:7,
                        useDHCP:true,
                        WiFiIP:"",
                        WiFiSubnet:"",
                        WiFiGateway:""
                      },
      NetworksInRange: [],
      commitWorking: false,
      scanWorking: false,
      WifiEncryption: {
        7: 'None',
        5: 'WEP',
        2: 'WPA/PSK',
        4: 'WPA2/PSK',
        8: 'WPA/WPA2'
      }
    }
  },
  computed: {
    // sort networks array by rssi = signal strength
    sortedNetworksInRange: function() {
      return this.NetworksInRange.sort(function (a, b) {
        // rssi is ranging from -100 to 0 with 0 being best signal quality
        if (a.rssi > b.rssi) {
          return -1;
        }
        if (a.rssi < b.rssi) {
          return 1;
        }

        // otherwise a.rssi equals b.rssi
        return 0;
      });
    }
  },
  methods: {
    // save the changes to the network settings
    commitChanges: function() {
      this.commitWorking=true;
      const loadingComponent = this.$loading.open();

      var newSettings={ap:  {ssid: this.SettingsNetwork.ESPName},
                       sta: {ssid: this.SettingsNetwork.WiFiName,
                             enc: this.SettingsNetwork.WiFiSecurity,
                             dhcp: this.SettingsNetwork.useDHCP,
                             ip: this.SettingsNetwork.WiFiIP,
                             gw: this.SettingsNetwork.WiFiGateway,
                             sn: this.SettingsNetwork.WiFiSubnet}};
      if (this.SettingsNetwork.APPassword_changed) {
        newSettings.ap.pwd=this.SettingsNetwork.APPassword;
      }
      if (this.SettingsNetwork.WiFiPassword_changed) {
        newSettings.sta.pwd=this.SettingsNetwork.WiFiPassword;
      }

      HTTP.post('/api/settings', newSettings
    ).then((response) => {
      this.$toast.open({
                    message: 'Settings saved successfully.',
                    type: 'is-success',
                    position: 'is-bottom'
                })
    }).catch((e) => {
      this.$toast.open({
                    message: 'Failed to save settings.',
                    type: 'is-danger',
                    position: 'is-bottom'
                })
    }).then(() => {
        this.commitWorking=false;
        loadingComponent.close();
      })
    },
    // call the api to get the wifi networks currently in range
    scanWifis: function() {
      this.scanWorking=true
      HTTP.get('/api/scanwifi'
    ).then((response) =>
      {
        if(response.status==200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json")))
        {
          this.NetworksInRange=response.data
        } else {
          this.NetworksInRange=[]
        }
      }).catch((e) =>
      {
        this.NetworksInRange=[];
        this.$toast.open({
                      message: 'Couldn\'t scan for wifi networks.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  });
      }).then(() => {
        this.scanWorking=false
      })
    },
    // select a specific wifi and make sure all settings of the previously selected wifi are reset
    selectWifi: function(network) {
      this.SettingsNetwork.WiFiName=network.ssid
      this.SettingsNetwork.WiFiSecurity=network.enc,
      this.SettingsNetwork.WiFiPassword=null,
      this.SettingsNetwork.WiFiPassword_changed=true;
      this.SettingsNetwork.useDHCP=true,
      this.SettingsNetwork.WiFiIP=null,
      this.SettingsNetwork.WiFiSubnet=null,
      this.SettingsNetwork.WiFiGateway=null

      this.NetworksInRange=[]
    },
    // calculate a percentage value for signal quality based on the Microsoft definition and calculate the correct icon number
    getIconNumber: function(rssi) {
      return Math.min(Math.max(Math.floor(2*(rssi+100)/100*4), 0), 4);
    }
  },
  // when the component is created, get the settings from the ESP and handle display.
  // passwords are not transmitted back by the ESP, but a dummy value is inserted
  created: function() {
    HTTP.get('/api/settings?type=*').then((response) =>
      {
        if(response.status==200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json")))
        {
          this.SettingsNetwork.ESPName=response.data["ap"]["ssid"];
          this.SettingsNetwork.APPassword="XXXXXXXX";
          this.SettingsNetwork.APPassword_changed=false;
          this.SettingsNetwork.WiFiName=response.data["sta"]["ssid"];
          this.SettingsNetwork.WiFiSecurity=response.data["sta"]["enc"];
          this.SettingsNetwork.WiFiPassword="XXXXXXXX";
          this.SettingsNetwork.WiFiPassword_changed=false;
          this.SettingsNetwork.useDHCP=response.data["sta"]["dhcp"];
          this.SettingsNetwork.WiFiIP=response.data["sta"]["ip"];
          this.SettingsNetwork.WiFiSubnet=response.data["sta"]["sn"];
          this.SettingsNetwork.WiFiGateway=response.data["sta"]["gw"];
        } else {
          this.SettingsNetwork={}
        }
      }).catch((e) =>
      {
        this.SettingsNetwork={};
        this.$toast.open({
                      message: 'Couldn\'t get settings from ESP.',
                      type: 'is-danger',
                      position: 'is-bottom'
                  });
      })
  }
}
</script>

<style scoped>

</style>
