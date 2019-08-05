# frozen_string_literal: true

require 'json'

module Readapt
  module Adapter
    # @!parse include Backport::Adapter

    # @todo Support multiple clients?
    @@stdout = nil
    @@stderr = nil
    @@client = nil
    @@inspector = nil
    @@debugger = nil

    def self.host debugger
      @@debugger = debugger
    end

    def self.attach inspector
      @@inspector = inspector
    end

    def self.attached?
      !!@@inspector
    end

    def self.connected?
      !!@@client
    end

    def format result
      write_line result.to_protocol.to_json
    end

    def opening
      # STDERR.puts "WOAH NELLY #{remote}"
      if !@@stdout
        STDERR.puts "Setting up the damn STDOUT"
        @@stdout = self
        remote[:client] = :stdout
      elsif !@@stderr
        @@stderr = self
        remote[:client] = :stderr
      else
        @@client = self
        remote[:client] = :user
        @@debugger.add_observer self
        @data_reader = DataReader.new
        @data_reader.set_message_handler do |message|
          process message
        end
      end
    end

    def closing
      @@debugger.delete_observer(self)
    end

    def receiving data
      if remote[:client] == :user
        @data_reader.receive data
      else
        STDERR.puts "Data from stdout"
        @@debugger.output data
      end
    end

    def update event, data
      json = if event == 'terminated'
        {
          type: 'event',
          event: 'terminated'
        }.to_json
      elsif event == 'output'
        {
          type: 'event',
          event: 'output',
          category: 'console',
          output: data
        }.to_json
      else
        {
          type: 'event',
          event: 'stopped',
          body: {
            reason: event,
            threadId: data
          }
        }.to_json
      end
      envelope = "Content-Length: #{json.bytesize}\r\n\r\n#{json}"
      write envelope
    end

    private

    # @param request [String]
    # @return [void]
    def process data
      # @todo Better solution than nil frames
      message = Message.process(data, (@@inspector || Inspector.new(@@debugger, nil)))
      if data['seq']
        json = {
          type: 'response',
          request_seq: data['seq'],
          success: true,
          command: data['command'],
          body: message.body
        }.to_json
        envelope = "Content-Length: #{json.bytesize}\r\n\r\n#{json}"
        write envelope
        @@inspector = nil unless @@inspector && @@inspector.control == :pause
        close if data['command'] == 'disconnect'
        return unless data['command'] == 'initialize'
        json = {
          type: 'event',
          event: 'initialized'
        }.to_json
        envelope = "Content-Length: #{json.bytesize}\r\n\r\n#{json}"
        write envelope
      end
    rescue Exception => e
      STDERR.puts e.message
      STDERR.puts e.backtrace
    end
  end

  class DataReader
    def initialize
      @in_header = true
      @content_length = 0
      @buffer = String.new
    end

    # Declare a block to be executed for each message received from the
    # client.
    #
    # @yieldparam [Hash] The message received from the client
    def set_message_handler &block
      @message_handler = block
    end

    # Process raw data received from the client. The data will be parsed
    # into messages based on the JSON-RPC protocol. Each message will be
    # passed to the block declared via set_message_handler. Incomplete data
    # will be buffered and subsequent data will be appended to the buffer.
    #
    # @param data [String]
    def receive data
      data.each_char do |char|
        @buffer.concat char
        if @in_header
          prepare_to_parse_message if @buffer.end_with?("\r\n\r\n")
        else
          parse_message_from_buffer if @buffer.bytesize == @content_length
        end
      end
    end

    private

    def prepare_to_parse_message
      @in_header = false
      @buffer.each_line do |line|
        parts = line.split(':').map(&:strip)
        if parts[0] == 'Content-Length'
          @content_length = parts[1].to_i
          break
        end
      end
      @buffer.clear
    end

    def parse_message_from_buffer
      begin
        msg = JSON.parse(@buffer)
        @message_handler.call msg unless @message_handler.nil?
      rescue JSON::ParserError => e
        Solargraph::Logging.logger.warn "Failed to parse request: #{e.message}"
        Solargraph::Logging.logger.debug "Buffer: #{@buffer}"
      ensure
        @buffer.clear
        @in_header = true
        @content_length = 0
      end
    end
  end
end
