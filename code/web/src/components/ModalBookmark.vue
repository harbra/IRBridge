<!--
This vue.js file contains a modal that can be used to create and modify bookmarks.
-->
<template>
<form v-on:submit.prevent="commitModal()">
<div class="card">
  <div class="card-header">
    <div class="card-header-title">
      {{dialogTitle}}
    </div>
  </div>
  <div class="card-content">
    <div class="field">
      <label class="label">Name</label>
      <p class="control">
        <input class="input" type="text" placeholder="Bookmark name" required v-model="myname" v-bind:class="{'is-danger': iserrorname}">
      </p>
    </div>

    <label class="label">Type</label>
    <div class="field has-addons">
      <div class="control">
        <div class="select">
          <select v-model="mytype">
              <option value="P">Protocol</option>
              <option value="R">RAW</option>
            </select>
        </div>
      </div>
      <div class="control" v-if="mytype==='P'">
        <div class="select">
          <select v-model="mycontentprot" key="prot">
              <option v-for="protocol in protocols" :value="protocols.indexOf(protocol)">{{protocol}}</option>
            </select>
        </div>
      </div>
      <div class="control" v-else>
        <div class="select">
          <select v-model="mycontentrawenc" key="rawenc">
            <option value='D'>dec.</option>
            <option value='H'>hex.</option>
          </select>
        </div>
      </div>
    </div>

      <template v-if="mytype==='R'">
        <label class="label">Frequency in Hz</label>
        <div class="field has-addons">
          <div class="control">
            <input class="input" type="number" min="0" v-model="mycontentfreq" v-bind:class="{'is-danger': iserrorcontentfreq}"/>
          </div>
          <div class="control">
            <a class="button" v-on:click="mycontentfreq=''" :disabled="mycontentfreq===''">
              <span class="icon">
                <i class="icon-times-circle"></i>
              </span>
            </a>
          </div>
        </div>
        <div class="field">
          <label class="label">Raw data ({{ mycontentrawenc=='D' ? "dec." : "hex." }}, comma separated)</label>
          <div class="control">
            <textarea class="textarea" type="text" placeholder="Enter comma separated raw data" v-model="mycontentraw" v-bind:class="{'is-danger': iserrorcontentraw}"></textarea>
          </div>
        </div>
      </template>
      <template v-else>
        <label class="label">Value</label>
        <div class="field has-addons">
          <a class="button is-static">
            0x
          </a>
          <div class="control">
            <input class="input" pattern="[a-fA-F\d]+" required v-model="mycontentvalue"  v-bind:class="{'is-danger': iserrorcontentvalue}"/>
          </div>
        </div>

        <label class="label"># of bits to send</label>
        <div class="field has-addons">
          <div class="control">
            <input class="input" type="number" min="0" v-model="mycontentbits"  v-bind:class="{'is-danger': iserrorcontentbits}"/>
          </div>
          <div class="control">
            <a class="button" v-on:click="mycontentbits=''" :disabled="mycontentbits===''">
              <span class="icon">
                <i class="icon-times-circle"></i>
              </span>
            </a>
          </div>
        </div>
      </template>

      <label class="label">Repeats</label>
      <div class="field has-addons">
        <div class="control">
          <input class="input" type="number" v-model="myrepeats"  v-bind:class="{'is-danger': iserrorrepeats}"/>
        </div>
        <div class="control">
          <a class="button" v-on:click="myrepeats=''" :disabled="myrepeats===''">
            <span class="icon">
              <i class="icon-times-circle"></i>
            </span>
          </a>
        </div>
      </div>
    </div>
    <div class="card-footer">
      <a class="card-footer-item button is-primary" v-on:click="commitModal()">
        <span class="icon">
          <i class="icon-save"></i>
        </span>
        <span>Save</span>
      </a>
      <a class="card-footer-item button is-danger" v-on:click="cancelModal()">
        <span class="icon">
          <i class="icon-remove"></i>
        </span>
        <span>Cancel</span>
      </a>
    </div>
</div>
</form>
</template>

<script>
import {
  HTTP
} from '../plugins/axios'

export default {
  name: 'ModalBookmark',
  data() {
    return {
      // the default data contents are populated from the props. If a property is not passed by props, it will be set to a default value
      myname: this.name,
      mytype: this.type,
      myrepeats: this.repeats,
      mycontentraw: (this.content.hasOwnProperty("raw") ? this.content.raw.map((val) => {
          return val.toString((this.content.hasOwnProperty("enc") ? this.content.enc : "D")=="D" ? 10 : 16);
        }).join() : ""),
      mycontentfreq: (this.content.hasOwnProperty("freq") ? this.content.freq : ""),
      mycontentprot: (this.content.hasOwnProperty("prot") ? this.content.prot : 1),
      mycontentvalue: (this.content.hasOwnProperty("value") ? this.content.value.toString(16) : 0),
      mycontentbits: (this.content.hasOwnProperty("bits") ? this.content.bits : ""),
      mycontentrawenc: (this.content.hasOwnProperty("enc") ? this.content.enc : "D"),
      iserrorname: false,
      iserrorrepeats: false,
      iserrorcontentraw: false,
      iserrorcontentfreq: false,
      iserrorcontentvalue: false,
      iserrorcontentbits: false
    }
  },
  props: {
    dialogTitle: { type: String,
                   required: true },
    name: String,
    type: {
      type: String,
      default: "P"
    },
    repeats: {
      type: [String,Number],
      default: ""
    },
    content: {
      type: Object,
      default: function() {
        return {};
      }
    },
    protocols: {
      type: Array,
      default: function() {
        return [];
      }
    }
  },
  methods: {
    /*
    Uses a RegEx to determin if a string contains a numeric value.
    The value can be given in decimal or hexadecimal.
    Returns either NaN or the number in numeric format.
    Hex numbers are converted to decimal when returned.
    */
    filterPosIntOrZero: function (value, hex) {
      hex = (typeof hex !== 'undefined') ?  hex : false;

      var re;
      if(hex) {
        re=/^([a-fA-F\d]+)$/;
      } else {
        re=/^([\d]+)$/;
      }

      if(re.test(value)) {
        return parseInt(value,(hex?16:10));
      }
      return NaN;
    },
    /*
    Validated the bookmark settings.
    */
    validate: function() {
      // name has to exist and be a valid string
      if(this.myname=="" || this.myname==undefined || this.myname==null) {
        this.iserrorname=true;
        this.$toast.open({
                      message: 'Please enter a name.',
                      type: 'is-warning',
                      position: 'is-top'
                  });
        return false;
      } else {
        this.iserrorname=false;
      }

      // repeats has to be numeric, given in decimal
      if(isNaN(this.filterPosIntOrZero(this.myrepeats)) && this.myrepeats!="") {
        this.iserrorrepeats=true;
        this.$toast.open({
                      message: 'Repeats has to be numeric (decimal).',
                      type: 'is-warning',
                      position: 'is-top'
                  });
        return false;
      } else {
        this.iserrorrepeats=false;
      }

      // validate bookmark contents based on bookmark type
      if(this.mytype=='P') { // validates bookmarks based on a given protocol
        if(isNaN(this.filterPosIntOrZero(this.mycontentvalue,true))) {
          this.iserrorcontentvalue=true;
          this.$toast.open({
                        message: 'Value has to be numeric (hexadecimal).',
                        type: 'is-warning',
                        position: 'is-top'
                    });
          return false;
        } else {
          this.iserrorcontentvalue=false;
        }

        if(isNaN(this.filterPosIntOrZero(this.mycontentbits)) && this.mycontentbits!="") {
          this.iserrorcontentbits=true;
          this.$toast.open({
                        message: 'Value has to be numeric (decimal).',
                        type: 'is-warning',
                        position: 'is-top'
                    });
          return false;
        } else {
          this.iserrorcontentbits=false;
        }
      } else if(this.mytype=='R') { // validates bookmarks based on raw data
        var re;
        if(this.mycontentrawenc=='D') {
          re=/^([\d]+,)*[\d]+$/g;
        } else {
          re=/^([a-fA-F\d]+,)*[a-fA-F\d]+$/g;
        }
        var found=this.mycontentraw.match(re);

        if(this.mycontentraw!="" && (found==null || found[0].length!=this.mycontentraw.length)) {
          this.iserrorcontentraw=true;
          this.$toast.open({
                        message: 'Raw data has to be comma separated numbers '+(this.mycontentrawenc=='D' ? '(decimal)' : '(hexadecimal).'),
                        type: 'is-warning',
                        position: 'is-top'
                    });
          return false;
        } else {
          this.iserrorcontentraw=false;
        }

        if(isNaN(this.filterPosIntOrZero(this.mycontentfreq)) && this.mycontentfreq!="") {
          this.iserrorcontentfreq=true;
          this.$toast.open({
                        message: 'Frequency has to be numeric (decimal).',
                        type: 'is-warning',
                        position: 'is-top'
                    });
          return false;
        } else {
          this.iserrorcontentfreq=false;
        }
      }

      return true;
    },
    cancelModal: function() {
      this.$emit('cancel');
    },
    commitModal: function() {
      // validate the current settings
      if(!this.validate()) {
        return;
      }

      // displays a spinning circle to show that the IRBridge is working
      const loadingComponent = this.$loading.open();

      // create an object to send to the ESP based on the settings in the modal
      var retval={
          name: this.myname,
          type: this.mytype,
          content: {}};

        if(this.myrepeats!="") {
          retval.repeats=this.filterPosIntOrZero(this.myrepeats);
        }

        if(this.mytype=='P') {
          retval.content.prot=this.mycontentprot;
          retval.content.value=this.filterPosIntOrZero(this.mycontentvalue,true);
          if(this.mycontentbits!="") {
              retval.content.bits=this.filterPosIntOrZero(this.mycontentbits);
          }
        } else {
          if(this.mycontentfreq!="") {
              retval.content.freq=this.filterPosIntOrZero(this.mycontentfreq);
          }

          var rawtemp=this.mycontentraw.split(',');
          var rawtempnum=[];
          for(var rawtempval of rawtemp) {
            var parseres=this.filterPosIntOrZero(rawtempval,(this.mycontentrawenc=='D' ? false : true));
            if(!isNaN(parseres)) {
              rawtempnum.push(parseres);
            }
          }
          retval.content.raw=rawtempnum;
        }

      // emit a function in the parent container with the composed bookmark object that is ready to be sent to the webservice
      // the object of the loading component is also passed to allow closing it from outside this code block
      this.$emit('commit',retval,loadingComponent);
    }
  }
}
</script>
