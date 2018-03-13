<!--
This vue.js file contains a modal that can be used to create and modify devices.
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
        <input class="input" type="text" placeholder="Device name" required v-model="myname" v-bind:class="{'is-danger': iserrorname}">
      </p>
    </div>

    <div class="level">
      <div class="level-left">
        <label class="label">Actions when switching on</label>
      </div>
      <div class="level-right">
        <div class="level-item">
          <div class="dropdown is-right is-hoverable">
            <div class="dropdown-trigger">
              <button class="button">
                <span class="icon">
                  <i class="icon-plus"></i>
                </span>
                <span>Add...</span>
                <span class="icon is-small">
                  <i class="icon-angle-down"></i>
                </span>
              </button>
            </div>
            <div class="dropdown-menu">
              <div class="dropdown-content">
                <a class="dropdown-item" v-on:click="myactions.on.push({'pause':null})">
                  Pause
                </a>
                <a class="dropdown-item" v-on:click="myactions.on.push({'bookmark':null})">
                  Bookmark
                </a>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <template v-if="'on' in myactions && myactions.on.length>0">
      <div class="field is-horizontal" v-for="(action,idx) in myactions.on">
        <div class="field-label is-normal">
          <label class="label" v-if="'bookmark' in action">Bookmark: </label>
          <label class="label" v-else-if="'pause' in action">Pause: </label>
        </div>
        <div class="field-body">
          <div class="field is-grouped">
              <div class="control select is-expanded" v-bind:class="{'is-danger': action.iserror}" v-if="'bookmark' in action">
                <select v-model="action.bookmark">
                  <option v-for="bookmark in bookmarks">{{bookmark}}</option>
                </select>
              </div>
              <input class="control input is-expanded" type="number" placeholder="seconds" required v-model="action.pause" v-bind:class="{'is-danger': action.iserror}" v-else-if="'pause' in action">
              <div class="control buttons has-addons">
                <a class="button is-small" v-on:click="swap(myactions.on,idx,idx+1)" :disabled="idx==myactions.on.length-1">
                  <span class="icon">
                    <i class="icon-angle-down"></i>
                  </span>
                </a>

                <a class="button is-small" v-on:click="swap(myactions.on,idx,idx-1)" :disabled="idx==0">
                  <span class="icon">
                    <i class="icon-angle-up"></i>
                  </span>
                </a>

                <a class="button is-small is-danger is-hidden-touch" v-on:click="myactions.on.splice(idx,1)">
                  <span class="icon">
                    <i class="icon-minus"></i>
                  </span>
                  <span>Delete</span>
                </a>
                <a class="button is-small is-danger is-hidden-desktop" v-on:click="myactions.on.splice(idx,1)">
                  <span class="icon">
                    <i class="icon-minus"></i>
                  </span>
                </a>
              </div>
          </div>
        </div>
      </div>
    </template>
    <template v-else>
      <div class="level">
        <div class="level-item has-text-centered">
          <div>
            <p class="title">No actions defined.</p>
          </div>
        </div>
      </div>
    </template>

    <div class="level">
      <div class="level-left">
        <label class="label">Actions when switching off</label>
      </div>
      <div class="level-right">
        <div class="level-item">
          <div class="dropdown is-right is-hoverable">
            <div class="dropdown-trigger">
              <button class="button">
                <span class="icon">
                  <i class="icon-plus"></i>
                </span>
                <span>Add...</span>
                <span class="icon is-small">
                  <i class="icon-angle-down"></i>
                </span>
              </button>
            </div>
            <div class="dropdown-menu">
              <div class="dropdown-content">
                <a class="dropdown-item" v-on:click="myactions.off.push({'pause':null})">Pause</a>
                <a class="dropdown-item" v-on:click="myactions.off.push({'bookmark':null})">Bookmark</a>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
    <template v-if="'on' in myactions && myactions.off.length>0">
      <div class="field is-horizontal" v-for="(action,idx) in myactions.off">
        <div class="field-label is-normal">
          <label class="label" v-if="'bookmark' in action">Bookmark: </label>
          <label class="label" v-else-if="'pause' in action">Pause: </label>
        </div>
        <div class="field-body">
          <div class="field is-grouped">
              <div class="control select is-expanded" v-bind:class="{'is-danger': action.iserror}" v-if="'bookmark' in action">
                <select v-model="action.bookmark">
                  <option v-for="bookmark in bookmarks">{{bookmark}}</option>
                </select>
              </div>
              <input class="control input is-expanded" type="number" placeholder="seconds" required v-model="action.pause" v-bind:class="{'is-danger': action.iserror}" v-else-if="'pause' in action">
              <div class="control buttons has-addons">
                <a class="button is-small" v-on:click="swap(myactions.off,idx,idx+1)" :disabled="idx==myactions.off.length-1">
                  <span class="icon">
                    <i class="icon-angle-down"></i>
                  </span>
                </a>

                <a class="button is-small" v-on:click="swap(myactions.off,idx,idx-1)" :disabled="idx==0">
                  <span class="icon">
                    <i class="icon-angle-up"></i>
                  </span>
                </a>

                <a class="button is-small is-danger is-hidden-touch" v-on:click="myactions.off.splice(idx,1)">
                  <span class="icon">
                    <i class="icon-minus"></i>
                  </span>
                  <span>Delete</span>
                </a>
                <a class="button is-small is-danger is-hidden-desktop" v-on:click="myactions.off.splice(idx,1)">
                  <span class="icon">
                    <i class="icon-minus"></i>
                  </span>
                </a>
              </div>
          </div>
        </div>
      </div>
    </template>
    <template v-else>
      <div class="level">
        <div class="level-item has-text-centered">
          <div>
            <p class="title">No actions defined.</p>
          </div>
        </div>
      </div>
    </template>
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
  name: 'ModalDevice',
  data() {
    return {
      myname: this.name,
      myactions: {"on": (this.actions.hasOwnProperty("on") ? this.actions.on : []),
                  "off": (this.actions.hasOwnProperty("off") ? this.actions.off : [])},
      iserrorname: false,
      bookmarks: []
    }
  },
  props: {
    dialogTitle: { type: String,
                   required: true },
    name: String,
    actions: {
      type: Object,
      default: function() {
        return {"on": [], "off": []};
      }
    }
  },
  methods: {
    // swaps two array elements
    swap: function(array, idx1, idx2) {
      var temp=array[idx2];
      array.splice(idx2,1,array[idx1]);
      array.splice(idx1,1,temp);
    },
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
    Validated the device settings.
    */
    validate: function() {
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

      // loop through all defined actions and make sure only existing bookmarks or pauses are included
      for(var key in this.myactions) {
        var actioncontainer=this.myactions[key];
        for(var i=0; i<actioncontainer.length; i++) {
          var action=actioncontainer[i];
          if('bookmark' in action) {
            if(this.bookmarks.indexOf(action.bookmark)==-1) {
              this.$set(action,'iserror',true);
              return false;
            } else {
              this.$set(action,'iserror',false);
            }
          } else if('pause' in action) {
            if(isNaN(this.filterPosIntOrZero(action.pause))) {
              this.$set(action,'iserror',true);
              return false;
            } else {
              action.pause=this.filterPosIntOrZero(action.pause);
              this.$set(action,'iserror',false);
            }
          }
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
          actions: {"on": this.myactions.on, "off": this.myactions.off}};

      // emit a function in the parent container with the composed bookmark object that is ready to be sent to the webservice
      // the object of the loading component is also passed to allow closing it from outside this code block
      this.$emit('commit',retval,loadingComponent);
    }
  },
  /*
  When the component is created, load the existing bookmark names to allow later validation of device configurations and to populate the select boxes
  */
  created: function() {
    var loadingComponent=this.$loading.open();

    HTTP.get('/api/bookmarks?short').then((response) => {
      if (response.status == 200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json"))) {
        this.bookmarks=response.data;
      } else {
        this.bookmarks = [];
      }
    }).catch((e) => {
      this.bookmarks = [];
      this.$toast.open({
        message: 'Couldn\'t get bookmarks from ESP.',
        type: 'is-danger',
        position: 'is-bottom'
      });
    }).then(() => {
      loadingComponent.close();
    })
  }
}
</script>
