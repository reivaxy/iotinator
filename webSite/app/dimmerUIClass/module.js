

console.log("DIMMER LOADED");

(function toto() {
  XIOT.loadJS("dimmerUIClass/slider/bootstrap-slider.min.js");
  XIOT.loadCSS("dimmerUIClass/slider/bootstrap-slider.min.css");
})();


var dimmerUIClass = {
  _count: 0,
  // TODO : Not used yet. When modules are loaded dynamically, this will be called by loader
  // init: function() {
  //   XIOT.loadJS("dimmerUIClass/slider/bootstrap-slider.min.js");
  //   XIOT.loadCSS("dimmerUIClass/slider/bootstrap-slider.min.css");
  // },

  // Model for the dimmer module
  Model: Backbone.Model.extend({
    defaults: function () {
      let model = {
        level: 0
      };
      return model;
    }
  }),

  // View for the dimmer module
  View: XIOT.View.extend({
    _count: 0,
    tagName: "div",
    template: _.template(
      '<div class="dimmerUIClassContent">' +
        '<input class="dimmerUIClassSlider" data-slider-id="dimmerUIClassSliderSlider" type="text" data-slider-min="0" ' +
            'data-slider-max="100" data-slider-step="1" data-slider-value="<%- level %>" data-slider-tooltip="hide"/>' +
        '<span class="levelText"><%= level %></span>' +
      '</div>'

    ),

    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "mouseup .slider-handle" : "level",
      "touchend .slider-handle" : "level",
      "touchleave .slider-handle" : "level"
    },

    level: function(e) {
      let value = this.$el.find('.dimmerUIClassSlider')[0].value;
      console.log("Slider: ", value);
      this.model.set('level', value);
      this.xiotSync(this.model);
    },
    render: function () {
      let that = this;
      let slider = this.$el.find('input.dimmerUIClassSlider').get(0);
      // first time the dom is built, need to add the slider
      if(slider == null) {
        this.$el.html(this.template(this.model.toJSON()));

          setTimeout(function () {
            that.$el.find('.dimmerUIClassSlider').slider({
              formatter: function (value) {
                return value;
              }
            });
            let justInserted = document.getElementById("dimmerUIClassSliderSlider");
            justInserted.id = 'dimmerUIClassSliderSlider_' + dimmerUIClass._count;

            dimmerUIClass._count++;
            return this;
          }, 100);
      } else {
        // If slider already exists, just update its value
        this.$el.find('input.dimmerUIClassSlider').slider({value: this.model.get('level')});
        this.$el.find('.levelText').text( this.model.get('level'));
      }
    }
  })

  
};